#include "../StarlingMonkey/builtins/web/performance.h"
#include "./builtins/fetch-event.h"
#include "./host-api/fastly.h"
#include "./host-api/host_api_fastly.h"
#include "extension-api.h"
#include "host_api.h"
#include <wasi/libc-environ.h>

using fastly::fetch_event::FetchEvent;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;

namespace fastly::runtime {

api::Engine *ENGINE;

// Install corresponds to Wizer time, so we configure the engine here
bool install(api::Engine *engine) {
  ENGINE = engine;
  engine->enable_module_mode(false);
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
    fastly::fetch_event::dispatch_fetch_event(fetch_event, &total_compute);
  } else {
    fastly::fetch_event::dispatch_fetch_event(fetch_event);
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

// Fastly uses main to then pull the backend request
int main(int argc, const char *argv[]) {
  host_api::Request req;

  if (fastly::req_body_downstream_get(&req.req.handle, &req.body.handle) != 0) {
    abort();
    return 1;
  }

  fastly::runtime::handle_incoming(req);

  return 0;
}
