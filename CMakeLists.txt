cmake_minimum_required(VERSION 3.10)

project (azul C CXX)
set (CMAKE_CXX_STANDARD 17)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
set_azul_platform_file()
include(${AZUL_PLATFORM_FILE})

add_subdirectory(${CMAKE_SOURCE_DIR}/src/utils)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/async)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/compute)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ipc/)

if (BUILD_TESTS AND NOT DISABLE_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/googletest)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/async/)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/compute/)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/ipc/)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/utils)
endif()
