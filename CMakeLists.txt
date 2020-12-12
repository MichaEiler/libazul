cmake_minimum_required(VERSION 3.14)

project (azul C CXX)
set (CMAKE_CXX_STANDARD 17)

option(LIBAZUL_WITH_IPC "Enable the build of the IPC component. (Not available on iOS and Android)" ON)
option(LIBAZUL_WITH_TESTS "Enable the compilation of all unit test projects. (Not available on iOS and Android)" ON)

include(${CMAKE_SOURCE_DIR}/cmake/platform.cmake)
set_global_compiler_options()

add_subdirectory(${CMAKE_SOURCE_DIR}/src/utils)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/async)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/compute)
if (LIBAZUL_WITH_IPC)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/ipc/)
endif()

if (LIBAZUL_WITH_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/googletest)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/async/)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/compute/)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/utils)
    if (LIBAZUL_WITH_IPC)
        add_subdirectory(${CMAKE_SOURCE_DIR}/tests/ipc/)
    endif()
endif()
