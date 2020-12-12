# fix debug flag set by androidndk provided toolchain file
string(REPLACE "-g" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
string(REPLACE "-g" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-g" "" CMAKE_ASM_FLAGS ${CMAKE_ASM_FLAGS})
set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_ASM_FLAGS_DEBUG     "${CMAKE_ASM_FLAGS_DEBUG} -g")

set(LIBAZUL_WITH_IPC OFF)
set(LIBAZUL_WITH_TESTS OFF)

function (set_global_compiler_options)
    set (options "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -Wpedantic -Wformat=2")

    if (CMAKE_BUILD_TYPE MATCHES DEBUG)
        set(options "${options} -g -O0")
    else()
        set(options "${options} -O2")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${options}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${options}" PARENT_SCOPE)
endfunction()

