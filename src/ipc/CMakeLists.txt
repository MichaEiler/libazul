if (SUPPORTS_IPC)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(PLATFORM_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/linux/*.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/linux/*.hpp")
    set(PLATFORM_DEPENDENCIES rt pthread)
elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(PLATFORM_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/darwin/*.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/drawin/*.hpp")
    set(PLATFORM_DEPENDENCIES sasl2)
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(PLATFORM_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.hpp")
else()
    message(FATAL_ERROR "This platform does not have an ipc implementation available.")
endif()

file (GLOB SOURCES ${PLATFORM_DIRS}
                   "${CMAKE_CURRENT_SOURCE_DIR}/detail/*.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/detail/*.hpp"
                   "${CMAKE_SOURCE_DIR}/include/impulso/ipc/*.hpp")

add_library (impulso_ipc ${SOURCES})
target_include_directories (impulso_ipc PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_include_directories (impulso_ipc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(impulso_ipc impulso_utils ${PLATFORM_DEPENDENCIES})

endif()
