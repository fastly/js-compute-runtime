#include <arpa/inet.h>

#include "builtin.h"
#include "extension-api.h"
#include "host_api.h"

namespace fastly::common {

JSString *ip_octets_to_js_string(JSContext *cx, host_api::HostBytes octets);

} // namespace fastly::common
