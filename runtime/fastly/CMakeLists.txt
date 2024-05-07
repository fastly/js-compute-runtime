cmake_minimum_required(VERSION 3.27)

include("../StarlingMonkey/cmake/add_as_subproject.cmake")

add_builtin(fastly::runtime SRC handler.cpp host-api/component/fastly_world_adapter.cpp)
add_builtin(fastly::fastly SRC builtins/fastly.cpp)
add_builtin(fastly::backend SRC builtins/backend.cpp)
add_builtin(fastly::fetch SRC builtins/fetch/fetch.cpp builtins/fetch/request-response.cpp builtins/fetch/headers.cpp)
add_builtin(fastly::cache_override SRC builtins/cache-override.cpp)
add_builtin(fastly::fetch_event SRC builtins/fetch-event.cpp)
add_builtin(fastly::cache_simple SRC builtins/cache-simple.cpp DEPENDENCIES OpenSSL)

project(FastlyJS)
