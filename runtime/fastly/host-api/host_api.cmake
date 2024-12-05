add_library(host_api STATIC
        ${HOST_API}/host_api.cpp
        ${HOST_API}/host_call.cpp
)

target_link_libraries(host_api PRIVATE spidermonkey)
target_include_directories(host_api PRIVATE include)
target_include_directories(host_api PRIVATE ${HOST_API})
target_include_directories(host_api PUBLIC ${HOST_API}/include)

