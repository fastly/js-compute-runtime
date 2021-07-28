#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>
#ifdef MEM_STATS
#include <string>
#endif

#include <wasi/libc-environ.h>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"

#include "js/CompilationAndEvaluation.h"
#include "js/Initialization.h"
#include "js/SourceText.h"

#pragma clang diagnostic pop

#include "js-compute-builtins.h"
#include "wizer.h"
#ifdef MEM_STATS
#include "memory-reporting.h"
#endif

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;

using JS::Value;

using JS::RootedValue;
using JS::RootedObject;
using JS::RootedString;

using JS::HandleValue;
using JS::HandleValueArray;
using JS::HandleObject;
using JS::MutableHandleValue;

using JS::PersistentRooted;

#ifdef MEM_STATS
size_t size_of_cb(const void* ptr) {
  return ptr ? sizeof(ptr) : 0;
}

static bool dump_mem_stats(JSContext* cx) {
  SimpleJSRuntimeStats rtStats(&size_of_cb);
  if (!JS::CollectRuntimeStats(cx, &rtStats, nullptr, false))
    return false;
  std::string rtPath = "rt";
  size_t rtTotal;
  ReportJSRuntimeExplicitTreeStats(rtStats, rtPath, nullptr, false, &rtTotal);

  printf("compartment counts: %zu sys, %zu usr\n", JS::SystemCompartmentCount(cx), JS::UserCompartmentCount(cx));
  printf("GC heap total: %zu\n", size_t(JS_GetGCParameter(cx, JSGC_TOTAL_CHUNKS)) * js::gc::ChunkSize);
  printf("GC heap unused: %zu\n", size_t(JS_GetGCParameter(cx, JSGC_UNUSED_CHUNKS)) * js::gc::ChunkSize);

  return true;
}
#endif // MEM_STATS

/* The class of the global object. */
static JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    &JS::DefaultGlobalClassOps
};

bool INITIALIZED = false;
JSContext* CONTEXT = nullptr;

JS::PersistentRootedObject GLOBAL;
JS::PersistentRootedObject unhandledRejectedPromises;

void gc_callback(JSContext* cx, JSGCStatus status, JS::GCReason reason, void* data) {
  if (debug_logging_enabled())
    printf("gc for reason %s, %s\n", JS::ExplainGCReason(reason), status ? "end" : "start");
}

static void rejection_tracker(JSContext* cx, bool mutedErrors, JS::HandleObject promise,
                              JS::PromiseRejectionHandlingState state, void* data)
{
  RootedValue promiseVal(cx, JS::ObjectValue(*promise));

  switch (state) {
    case JS::PromiseRejectionHandlingState::Unhandled: {
      if (!JS::SetAdd(cx, unhandledRejectedPromises, promiseVal)) {
        // Note: we unconditionally print these, since they almost always indicate serious bugs.
        fprintf(stderr, "Adding an unhandled rejected promise to the promise "
                        "rejection tracker failed");
      }
      return;
    }
    case JS::PromiseRejectionHandlingState::Handled: {
      bool deleted = false;
      if (!JS::SetDelete(cx, unhandledRejectedPromises, promiseVal, &deleted)) {
        // Note: we unconditionally print these, since they almost always indicate serious bugs.
        fprintf(stderr, "Removing an handled rejected promise from the promise "
                        "rejection tracker failed");
      }
    }
  }
}

bool init_js() {
  JS_Init();

  JSContext *cx = JS_NewContext(JS::DefaultHeapMaxBytes);
  if (!cx)
      return false;
  if (!js::UseInternalJobQueues(cx) || !JS::InitSelfHostedCode(cx))
      return false;

  // TODO: check if we should set a different creation zone.
  JS::RealmOptions options;
  options.creationOptions()
    .setStreamsEnabled(true)
    .setReadableByteStreamsEnabled(true)
    .setBYOBStreamReadersEnabled(true)
    .setReadableStreamPipeToEnabled(true)
    .setWritableStreamsEnabled(true)
    .setIteratorHelpersEnabled(true)
    .setWeakRefsEnabled(JS::WeakRefSpecifier::EnabledWithoutCleanupSome);

  JS::DisableIncrementalGC(cx);
  // JS_SetGCParameter(cx, JSGC_MAX_EMPTY_CHUNK_COUNT, 1);

  RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook,
                                             options));
  if (!global)
      return false;

  JSAutoRealm ar(cx, global);
  if (!JS::InitRealmStandardClasses(cx))
    return false;

  JS::SetPromiseRejectionTrackerCallback(cx, rejection_tracker);


  CONTEXT = cx;
  GLOBAL.init(cx, global);
  unhandledRejectedPromises.init(cx, JS::NewSetObject(cx));
  if (!unhandledRejectedPromises)
    return false;

  return true;
}

static bool report_unhandled_promise_rejections(JSContext* cx) {
  RootedValue iterable(cx);
  if (!JS::SetValues(cx, unhandledRejectedPromises, &iterable))
    return false;

  JS::ForOfIterator it(cx);
  if (!it.init(iterable))
    return false;

  RootedValue promise_val(cx);
  RootedObject promise(cx);
  while (true) {
    bool done;
    if (!it.next(&promise_val, &done))
      return false;

    if (done)
      break;

    promise = &promise_val.toObject();
    // Note: we unconditionally print these, since they almost always indicate serious bugs.
    fprintf(stderr, "Promise rejected but never handled: ");
    dump_value(cx, JS::GetPromiseResult(promise), stderr);
  }

  return true;
}

static void abort(JSContext* cx, const char* description) {
  // Note: we unconditionally print messages here, since they almost always indicate serious bugs.
  if (JS_IsExceptionPending(cx)) {
    JS::ExceptionStack exception(cx);
    if (!JS::GetPendingExceptionStack(cx, &exception)) {
      fprintf(stderr, "Error: exception pending after %s, but got another error "
              "when trying to retrieve it. Aborting.\n", description);
    } else {
      fprintf(stderr, "Exception while %s: ", description);
      dump_value(cx, exception.exception(), stderr);
      print_stack(cx, exception.stack(), stderr);
    }
  } else {
    fprintf(stderr, "Error while %s, but no exception is pending. "
            "Aborting, since that doesn't seem recoverable at all.\n", description);
  }

  if (JS::SetSize(cx, unhandledRejectedPromises) > 0) {
    fprintf(stderr,
            "Additionally, some promises were rejected, but the rejection never handled:\n");
    report_unhandled_promise_rejections(cx);
  }

  // Respond with status `500` if no response was ever sent.
  if (INITIALIZED && !did_send_response())
    send_error_response(cx);

  fflush(stderr);
  exit(1);
}

bool eval_stdin(JSContext* cx, MutableHandleValue result) {
  char* code = NULL;
  size_t len = 0;
  if (getdelim(&code, &len, EOF, stdin) < 0) {
      return false;
  }

  JS::CompileOptions opts(cx);
  opts.setForceFullParse();
  // TODO: investigate passing a filename to Wizer and using that here to improve diagnostics.
  // TODO: furthermore, investigate whether Wizer by now allows us to pass an actual path
  // and open that, instead of having to redirect `stdin` for a subprocess of `js-compute-runtime`.
  opts.setFileAndLine("<stdin>", 1);

  JS::SourceText<mozilla::Utf8Unit> srcBuf;
  if (!srcBuf.init(cx, code, strlen(code), JS::SourceOwnership::TakeOwnership)) {
      return false;
  }

  JS::RootedScript script(cx);
  {
    // Disabling GGC during compilation seems to slightly reduce the number of
    // pages touched post-deploy.
    // (Whereas disabling it during execution below meaningfully increases it,
    // which is why this is scoped to just compilation.)
    JS::AutoDisableGenerationalGC noGGC(cx);
    script = JS::Compile(cx, opts, srcBuf);
    if (!script) return false;
  }

  // TODO: verify that it's better to perform a shrinking GC here, as manual testing
  // indicates. Running a shrinking GC here causes *fewer* 4kb pages to be written to when
  // processing a request, at least for one fairly large input script.
  //
  // A hypothesis for why this is the case could be that the objects allocated by parsing
  // the script (but not evaluating it) tend to be read-only, so optimizing them for
  // compactness makes sense and doesn't fragment writes later on.
  JS::PrepareForFullGC(cx);
  JS::NonIncrementalGC(cx, JS::GCOptions::Shrink, JS::GCReason::API);

  if (!JS_ExecuteScript(cx, script, result))
    return false;

  // TODO: check if it makes sense to increase the empty chunk count *before* running GC like this.
  // The working theory is that otherwise the engine might mark chunk pages as free that then later
  // the allocator doesn't turn into chunks without further fragmentation. But that might be wrong.
  // JS_SetGCParameter(cx, JSGC_MAX_EMPTY_CHUNK_COUNT, 10);

  // TODO: verify that it's better to *not* perform a shrinking GC here, as manual testing
  // indicates. Running a shrinking GC here causes *more* 4kb pages to be written to when
  // processing a request, at least for one fairly large input script.
  //
  // A hypothesis for why this is the case could be that most writes are to object kinds that are
  // initially allocated in the same vicinity, but that the shrinking GC causes them to be
  // intermingled with other objects. I.e., writes become more fragmented due to the shrinking GC.
  JS::PrepareForFullGC(cx);
  JS::NonIncrementalGC(cx, JS::GCOptions::Normal, JS::GCReason::API);

  // Ignore the first GC, but then print all others, because ideally GCs
  // should be rare, and developers should know about them.
  // TODO: consider exposing a way to parameterize this, and/or specifying a dedicated log target
  // for telemetry messages like this.
  JS_SetGCCallback(cx, gc_callback, nullptr);

  return true;
}

static PersistentRooted<Value> HANDLE_REQUEST;
static PersistentRooted<JSObject*> REQUEST_EVENT;

static bool addEventListener(JSContext* cx, unsigned argc, Value* vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "addEventListener", 2))
    return false;

  size_t event_len;
  JS::UniqueChars event_chars = encode(cx, args[0], &event_len);
  if (!event_chars) return false;

  if (strncmp(event_chars.get(), "fetch", event_len)) {
    fprintf(stderr, "Error: addEventListener only supports the event 'fetch' right now, "
            "but got event '%s'\n", event_chars.get());
    exit(1);
  }

  if (!HANDLE_REQUEST.isUndefined()) {
    fprintf(stderr, "Error: Can't add more than one listener for the 'fetch' event\n");
    exit(1);
  }

  RootedValue val(cx, args[1]);
  if (!val.isObject() || !JS_ObjectIsFunction(&val.toObject())) {
    fprintf(stderr, "Error: addEventListener: Argument 2 is not a function.\n");
    exit(1);
  }

  HANDLE_REQUEST.init(cx, val);
  return true;
}

void init() {
    assert(!INITIALIZED);

  if (!init_js())
    exit(1);

  JSContext* cx = CONTEXT;
  RootedObject global(cx, GLOBAL);
  JSAutoRealm ar(cx, global);

  define_fastly_sys(cx, global);
  if (!JS_DefineFunction(cx, global, "addEventListener", addEventListener, 2, 0))
    exit(1);

  RootedValue result(cx);
  if (!eval_stdin(cx, &result))
    abort(cx, "evaluating JS");

  if (!result.isUndefined()) {
    if (!dump_value(cx, result, stdout))
      exit(1);
  }

  REQUEST_EVENT.init(cx, create_downstream_request_event(cx));
  if (!REQUEST_EVENT.get())
    exit(1);

  if (HANDLE_REQUEST.isUndefined()) {
    RootedValue val(cx);
    if (!JS_GetProperty(cx, global, "handleRequest", &val) ||
        !val.isObject() || !JS_ObjectIsFunction(&val.toObject()))
    {
      // The error message only mentions `addEventListener`, even though we also support
      // a `handleRequest` top-level function as an alternative. We're treating the latter
      // as undocumented functionality for the time being.
      fprintf(stderr, "Error: no `fetch` event handler registered during initialization. "
                      "Make sure to call `addEventListener('fetch', your_handler)`.\n");
      exit(1);
    }

    HANDLE_REQUEST.init(cx, val);
  }

  fflush(stdout);
  fflush(stderr);

  // Define this to print a simple memory usage report.
#ifdef MEM_STATS
  dump_mem_stats(cx);
#endif

  INITIALIZED = true;
}

WIZER_INIT(init);

static void call_request_handler(JSContext* cx, double* total_compute) {
  auto pre_handler = system_clock::now();

  RootedValue result(cx);
  RootedValue val(cx);
  val.setObject(*REQUEST_EVENT);
  HandleValueArray argsv = HandleValueArray(val);

  if (!JS_CallFunctionValue(cx, GLOBAL, HANDLE_REQUEST, argsv, &val))
    abort(cx, "calling request handler");

  double diff = duration_cast<microseconds>(system_clock::now() - pre_handler).count();
  *total_compute += diff;
  if (debug_logging_enabled())
    printf("Request handler took %fms\n", diff / 1000);
}

static void process_pending_jobs(JSContext* cx, double* total_compute) {
  auto pre_reactions = system_clock::now();
  if (debug_logging_enabled()) {
    printf("Running promise reactions\n");
    fflush(stdout);
  }

  while (js::HasJobsPending(cx)) {
    js::RunJobs(cx);

    if (JS_IsExceptionPending(cx))
      abort(cx, "running Promise reactions");
  }

  double diff = duration_cast<microseconds>(system_clock::now() - pre_reactions).count();
  *total_compute += diff;
  if (debug_logging_enabled())
    printf("Running promise reactions took %fms\n", diff / 1000);
}

static void wait_for_backends(JSContext* cx, double* total_compute) {
  if (!has_pending_requests())
    return;

  auto pre_requests = system_clock::now();
  if (debug_logging_enabled()) {
    printf("Waiting for backends ...\n");
    fflush(stdout);
  }

  if (!process_network_io(cx))
    abort(cx, "processing network requests");

  double diff = duration_cast<microseconds>(system_clock::now() - pre_requests).count();
  if (debug_logging_enabled())
    printf("Done, waited for %fms\n", diff / 1000);
}

int main(int argc, const char *argv[]) {
  if (!INITIALIZED) {
    init();
    assert(INITIALIZED);
      // fprintf(stderr, "js.wasm must be initialized with a JS source file using Wizer\n");
      // exit(-1);
  }

  double total_compute = 0;
  auto start = system_clock::now();

  __wasilibc_initialize_environ();

  if (debug_logging_enabled()) {
    printf("Running JS handleRequest function for C@E service version %s\n",
           getenv("FASTLY_SERVICE_VERSION"));
    fflush(stdout);
  }

  JSContext* cx = CONTEXT;
  JSAutoRealm ar(cx, GLOBAL);
  js::ResetMathRandomSeed(cx);

  call_request_handler(cx, &total_compute);

  // Loop until no more resolved promises or backend requests are pending.
  if (debug_logging_enabled()) {
    printf("Start processing async jobs ...\n");
    fflush(stdout);
  }

  do {
    process_pending_jobs(cx, &total_compute);
    wait_for_backends(cx, &total_compute);
  } while (js::HasJobsPending(cx) || has_pending_requests());

  if (JS::SetSize(cx, unhandledRejectedPromises) > 0) {
    report_unhandled_promise_rejections(cx);

    // Respond with status `500` if any promise rejections were left unhandled
    // and no response was ever sent.
    if (!did_send_response())
      send_error_response(cx);
  }

  auto end = system_clock::now();
  double diff = duration_cast<microseconds>(end - start).count();
  if (debug_logging_enabled()) {
    printf("Done. Total request processing time: %fms. Total compute time: %fms\n",
           diff / 1000, total_compute / 1000);
  }

  // Note: we deliberately skip shutdown, because it takes quite a while,
  // and serves no purpose for us.
  // TODO: investigate also skipping the destructors deliberately run in wizer.h.
  // GLOBAL = nullptr;
  // CONTEXT = nullptr;
  // JS_DestroyContext(cx);
  // JS_ShutDown();

  return 0;
}
