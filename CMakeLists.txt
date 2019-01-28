cmake_minimum_required(VERSION 3.10)

project (impulso C CXX)
set (CMAKE_CXX_STANDARD 14)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
set_impulso_platform_file()
include(${IMPULSO_PLATFORM_FILE})

add_subdirectory(./src/utils)
add_subdirectory(./src/ipc/)

if (BUILD_TESTS AND NOT DISABLE_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/googletest)
    add_subdirectory(./tests/ipc/)
    add_subdirectory(./tests/utils)
endif()
