cmake_minimum_required(VERSION 3.27)

include("../StarlingMonkey/cmake/add_as_subproject.cmake")

add_builtin(fastly::runtime SRC handler.cpp common/ip_octets_to_js_string.cpp common/normalize_http_method.cpp common/validations.cpp)
add_builtin(fastly::cache_simple SRC builtins/cache-simple.cpp DEPENDENCIES OpenSSL)
add_builtin(fastly::fastly SRC builtins/fastly.cpp)
add_builtin(fastly::backend SRC builtins/backend.cpp)
add_builtin(fastly::body SRC builtins/body.cpp)
add_builtin(fastly::cache_core SRC builtins/cache-core.cpp)
add_builtin(fastly::kv_store SRC builtins/kv-store.cpp)
add_builtin(fastly::logger SRC builtins/logger.cpp)
add_builtin(fastly::device SRC builtins/device.cpp)
add_builtin(fastly::dictionary SRC builtins/dictionary.cpp)
add_builtin(fastly::edge_rate_limiter SRC builtins/edge-rate-limiter.cpp)
add_builtin(fastly::config_store SRC builtins/config-store.cpp)
add_builtin(fastly::secret_store SRC builtins/secret-store.cpp)
add_builtin(fastly::fetch SRC builtins/fetch/fetch.cpp builtins/fetch/request-response.cpp builtins/fetch/request-response.cpp ../StarlingMonkey/builtins/web/fetch/headers.cpp)
add_builtin(fastly::cache_override SRC builtins/cache-override.cpp)
add_builtin(fastly::fetch_event SRC builtins/fetch-event.cpp DEPENDENCIES OpenSSL)

add_compile_definitions(PUBLIC RUNTIME_VERSION=${RUNTIME_VERSION})

project(FastlyJS)
