cmake_minimum_required(VERSION 3.27)

#FIXME(1243)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../rust-toolchain.toml" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../StarlingMonkey")

include("../StarlingMonkey/cmake/add_as_subproject.cmake")

add_builtin(
  fastly::runtime
  SRC
    handler.cpp
    common/ip_octets_to_js_string.cpp
    common/normalize_http_method.cpp
    common/validations.cpp)

add_builtin(fastly::cache_simple
  SRC
    builtins/cache-simple.cpp
  DEPENDENCIES
    OpenSSL)

add_builtin(fastly::fastly SRC builtins/fastly.cpp)
add_builtin(fastly::acl SRC builtins/acl.cpp)
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
add_builtin(fastly::image_optimizer SRC builtins/image-optimizer.cpp)
add_builtin(fastly::shielding SRC builtins/shielding.cpp)

add_builtin(fastly::fetch
  SRC
    builtins/fetch/fetch.cpp
    builtins/fetch/request-response.cpp
    builtins/fetch/request-response.cpp
    ../StarlingMonkey/builtins/web/fetch/headers.cpp
    ../StarlingMonkey/builtins/web/fetch/fetch-utils.cpp
  DEPENDENCIES
    fmt)

add_builtin(fastly::cache_override SRC builtins/cache-override.cpp)

add_builtin(fastly::fetch_event
  SRC
    builtins/fetch-event.cpp
  DEPENDENCIES
    OpenSSL)

add_rust_lib(lol_html_c_api "${CMAKE_CURRENT_SOURCE_DIR}/crates/rust-lol-html" "\"capi\"")

add_builtin(fastly::html_rewriter
  SRC
    builtins/html-rewriter.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/crates/rust-lol-html/include
)

add_compile_definitions(PUBLIC RUNTIME_VERSION=${RUNTIME_VERSION})

project(FastlyJS)
