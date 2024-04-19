cmake_minimum_required(VERSION 3.27)

include("../StarlingMonkey/cmake/add_as_subproject.cmake")

add_builtin(fastly::runtime SRC handler.cpp host-api/component/fastly_world_adapter.cpp)
add_builtin(fastly::fastly SRC builtins/fastly.cpp)
add_builtin(fastly::fetch SRC builtins/fetch/request-response.cpp builtins/fetch/headers.cpp)
add_builtin(fastly::fetch_event SRC builtins/fetch-event.cpp)

project(FastlyJS)
