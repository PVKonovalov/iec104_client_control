cmake_minimum_required(VERSION 3.30)
project(iec104_client_control)

set(ABSL_PROPAGATE_CXX_STD ON)
set(CMAKE_CXX_STANDARD 20)
set(LIB_60870 vendor/lib60870-C)

find_package(Threads REQUIRED)

add_executable(iec104_client_control main.cpp)

include_directories(
        include
        ${LIB_60870}/src/inc/api
        ${LIB_60870}/src/hal/inc
)

add_subdirectory(${LIB_60870})

link_directories(${CMAKE_BINARY_DIR})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(COCOA_LIBRARY Cocoa ONLY)
    find_library(SECURITY_LIBRARY Security ONLY)
    find_library(RESOLV_LIBRARY Resolv ONLY)
    target_link_libraries(
            iec104_client_control
            PUBLIC Threads::Threads
            lib60870
            ${COCOA_LIBRARY}
            ${SECURITY_LIBRARY}
            ${RESOLV_LIBRARY}
    )
else ()
    target_link_libraries(
            iec104_client_control
            PUBLIC Threads::Threads
            lib60870
    )
endif ()
