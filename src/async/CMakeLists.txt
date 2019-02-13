if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(PLATFORM_DEPENDENCIES rt pthread)
elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
else()
    message(FATAL_ERROR "This platform does not have an async implementation available.")
endif()

file (GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/impulso/async/*.hpp")

add_library (impulso_async INTERFACE)
target_sources(impulso_async INTERFACE ${SOURCES})
target_link_libraries(impulso_async INTERFACE ${PLATFORM_DEPENDENCIES})

if (MSVC)
add_custom_target(impulso_async_headers SOURCES ${SOURCES})
endif()