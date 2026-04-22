cmake_minimum_required(VERSION 3.27)

#FIXME(1243)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../rust-toolchain.toml" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../StarlingMonkey")

# Apply any local patches to StarlingMonkey before it is included as a subproject
file(GLOB SM_PATCHES "${CMAKE_CURRENT_SOURCE_DIR}/patches/starlingmonkey-*.patch")
foreach(patch IN LISTS SM_PATCHES)
  execute_process(
    COMMAND git apply --check "${patch}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../StarlingMonkey"
    RESULT_VARIABLE patch_check_result
    OUTPUT_QUIET ERROR_QUIET
  )
  if(patch_check_result EQUAL 0)
    execute_process(
      COMMAND git apply "${patch}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../StarlingMonkey"
      RESULT_VARIABLE patch_result
    )
    if(NOT patch_result EQUAL 0)
      message(FATAL_ERROR "Failed to apply StarlingMonkey patch: ${patch}")
    endif()
  endif()
endforeach()

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

if(DEFINED FASTLY_GC_FREQUENCY)
  add_compile_definitions(PUBLIC FASTLY_GC_FREQUENCY=${FASTLY_GC_FREQUENCY})
endif()

project(FastlyJS)
