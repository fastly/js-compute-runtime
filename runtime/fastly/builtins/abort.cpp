
#include "../../StarlingMonkey/builtins/web/abort/abort-signal.h"

// StarlingMonkey's event builtin depends on parts of the abort builtin, but we don't support aborting
// fetches yet, so we don't build the abort builtin at all. To get around this, we stub the relevant
// abort signal methods that the event builtin depends on.

namespace builtins::web::abort {
bool AbortSignal::add_algorithm(JSObject *, js::UniquePtr<AbortAlgorithm>) {
    return true;
}

bool AbortSignal::is_aborted(JSObject *) {
    return false;
}

void AbortSignal::finalize(JS::GCContext *, JSObject *){}
void AbortSignal::trace(JSTracer *, JSObject *){}
}
namespace fastly::abort {
bool install(api::Engine *engine) {
  return true;
}
}