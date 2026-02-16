#include "../StarlingMonkey/builtins/web/performance.h"
#include "./builtins/fetch-event.h"
#include "./builtins/fastly.h"
#include "./host-api/fastly.h"
#include "./host-api/host_api_fastly.h"
#include "extension-api.h"
#include "host_api.h"
#include <wasi/libc-environ.h>
#include <chrono>

using fastly::fetch_event::FetchEvent;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;

namespace fastly::runtime {

api::Engine *ENGINE;

// Install corresponds to Wizer time, so we configure the engine here
bool install(api::Engine *engine) {
  ENGINE = engine;
  return true;
}

void handle_incoming(host_api::Request req) {
  builtins::web::performance::Performance::timeOrigin.emplace(
      std::chrono::high_resolution_clock::now());

  double total_compute = 0;
  std::chrono::system_clock::time_point start;
  if (ENGINE->debug_logging_enabled()) {
    start = system_clock::now();
  }

  __wasilibc_initialize_environ();

  if (ENGINE->debug_logging_enabled()) {
    printf("Running JS handleRequest function for Fastly Compute service version %s\n",
           getenv("FASTLY_SERVICE_VERSION"));
    fflush(stdout);
  }

  HandleObject fetch_event = FetchEvent::instance();
  if (!FetchEvent::init_request(ENGINE->cx(), fetch_event, req.req, req.body)) {
    ENGINE->dump_pending_exception("initialization of FetchEvent");
    return;
  }

  if (ENGINE->debug_logging_enabled()) {
    fetch_event::dispatch_fetch_event(fetch_event, &total_compute);
  } else {
    fetch_event::dispatch_fetch_event(fetch_event);
  }

  // Loop until no more resolved promises or backend requests are pending.
  if (ENGINE->debug_logging_enabled()) {
    printf("Start processing async jobs ...\n");
    fflush(stdout);
  }

  bool success = ENGINE->run_event_loop();

  if (JS_IsExceptionPending(ENGINE->cx())) {
    ENGINE->dump_pending_exception("evaluating code");
  } else if (!success) {
    if (ENGINE->has_pending_async_tasks()) {
      fprintf(stderr, "Warning: JS event loop terminated with async tasks pending. "
                      "Use FetchEvent#waitUntil to extend the service's lifetime "
                      "if needed.\n");
    } else {
      fprintf(stderr, "Warning: JS event loop terminated without completing the request.\n");
    }
  }

  if (ENGINE->debug_logging_enabled() && ENGINE->has_pending_async_tasks()) {
    fprintf(stderr, "Warming: JS event loop terminated with async tasks pending. "
                    "Use FetchEvent#waitUntil to extend the service's lifetime "
                    "if needed.\n");
    return;
  }

  // Respond with status `500` if no response was ever sent.
  if (!FetchEvent::response_started(fetch_event)) {
    FetchEvent::respondWithError(ENGINE->cx(), fetch_event);
    return;
  }

  if (ENGINE->debug_logging_enabled()) {
    auto end = system_clock::now();
    double diff = duration_cast<microseconds>(end - start).count();
    printf("Done. Total request processing time: %fms. Total compute time: %fms\n", diff / 1000,
           total_compute / 1000);
  }
  return;
}

} // namespace fastly::runtime

int main(int argc, const char *argv[]) {
  using fastly::fastly::Fastly;
  using fastly::runtime::ENGINE;
  Fastly::reusableSandboxOptions.freeze();

  host_api::HttpReqPromise::DownstreamNextOptions options;
  if (Fastly::reusableSandboxOptions.between_request_timeout()) {
    options.timeout_ms = static_cast<uint32_t>(Fastly::reusableSandboxOptions.between_request_timeout().value().count());
  }

  auto req = host_api::Request::downstream_get();
  if (req.is_err()) {
      HANDLE_ERROR(ENGINE->cx(), *req.to_err());
      return -1;
  }

  const auto max_requests = Fastly::reusableSandboxOptions.max_requests().value_or(1);
  std::size_t requests_handled = 0;
  const auto start_time = std::chrono::high_resolution_clock::now();
  while (true) {
    fastly::runtime::handle_incoming(req.unwrap());
    requests_handled++;

    // Check if we should exit based on configured max requests
    // Note that a max request value of 0 means unlimited,
    // so we only check the max requests condition if max_requests is greater than 0.
    if (max_requests > 0 && requests_handled >= max_requests) {
      if (fastly::runtime::ENGINE->debug_logging_enabled()) {
        printf("Max requests handled (%zu), exiting process.\n", requests_handled);
      }
      break;
    }

    // Check if we should exit based on configured sandbox timeout
    if (Fastly::reusableSandboxOptions.sandbox_timeout()) {
      auto now = std::chrono::high_resolution_clock::now();
      auto elapsed = now - start_time;
      if (elapsed >= Fastly::reusableSandboxOptions.sandbox_timeout().value()) {
        if (fastly::runtime::ENGINE->debug_logging_enabled()) {
          printf("Sandbox timeout reached (%llu ms), exiting process.\n",
                 std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        }
        break;
      }
    }

    // Check if we should exit based on configured max memory usage
    if (Fastly::reusableSandboxOptions.max_memory_mib()) {
      uint32_t heap_mib;
      if (fastly::compute_get_heap_mib(&heap_mib) != 0) {
        // If we fail to get heap memory usage, log a warning but continue anyway since this isn't a critical failure.
        if (fastly::runtime::ENGINE->debug_logging_enabled()) {
          printf("Failed to get heap memory usage, continuing anyway.\n");
        }
      }
      else if (heap_mib >= Fastly::reusableSandboxOptions.max_memory_mib().value()) {
        if (fastly::runtime::ENGINE->debug_logging_enabled()) {
          printf("Max memory exceeded (heap usage: %u MiB, max: %u MiB), exiting process.\n",
                 heap_mib, Fastly::reusableSandboxOptions.max_memory_mib().value());
        }
        break;
      }
    }

    auto next = host_api::HttpReqPromise::downstream_next(options);
    if (next.is_err()) {
      HANDLE_ERROR(ENGINE->cx(), *next.to_err());
      return -1;
    }
    
    req = next.unwrap().wait();
    if (req.is_err()) {
      HANDLE_ERROR(ENGINE->cx(), *req.to_err());
      return -1;
    }

    // The FetchEvent instance is a singleton that we re-initialize here. It's originally initialized during engine setup.
    if (!FetchEvent::reset(fastly::runtime::ENGINE->cx(), FetchEvent::instance())) {
      fprintf(stderr, "Failed to reset FetchEvent instance for new request, exiting process.\n");
      return -1;
    }
  }

  if (fastly::runtime::ENGINE->debug_logging_enabled()) {
    printf("Exiting process after handling %zu requests.\n", requests_handled);
    fflush(stdout);
  }

  return 0;
}
