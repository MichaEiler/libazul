set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

function (set_global_compiler_options)
    set(options "/W4")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${options}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${options}" PARENT_SCOPE)
endfunction()

