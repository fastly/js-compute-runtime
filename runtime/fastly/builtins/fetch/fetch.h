#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "request-response.h"
#include <optional>

namespace fastly::fetch {
extern api::Engine *ENGINE;
extern bool http_caching_unsupported;

// Try to serve a stale-if-error response when an error occurs.
// Returns the stale response if available, or std::nullopt if not.
std::optional<JSObject *> try_serve_stale_if_error(JSContext *cx, 
                                                    JS::HandleObject request_or_response,
                                                    JS::HandleValue error_val);
} // namespace fastly::fetch
