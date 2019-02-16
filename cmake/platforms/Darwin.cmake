set(BUILD_TESTS TRUE)
set(SUPPORTS_IPC TRUE)

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

set_global_compiler_options()

