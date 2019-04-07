if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(PLATFORM_DEPENDENCIES rt pthread)
elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
elseif(CMAKE_SYSTEM_NAME MATCHES Android)
elseif(CMAKE_SYSTEM_NAME MATCHES iOS)
else()
    message(FATAL_ERROR "This platform does not have an async implementation available.")
endif()

file (GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/azul/async/*.hpp")

add_library (azul_async INTERFACE)
target_sources(azul_async INTERFACE ${SOURCES})
target_link_libraries(azul_async INTERFACE ${PLATFORM_DEPENDENCIES})

if (MSVC)
add_custom_target(azul_async_headers SOURCES ${SOURCES})
endif()
