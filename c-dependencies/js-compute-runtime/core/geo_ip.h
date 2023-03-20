#ifndef JS_COMPUTE_RUNTIME_GEO_IP_H
#define JS_COMPUTE_RUNTIME_GEO_IP_H

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"

#include "jsapi.h"
#include "jsfriendapi.h"

#pragma clang diagnostic pop

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str);

#endif
