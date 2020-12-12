if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(PLATFORM_DEPENDENCIES rt pthread)
endif()

file (GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/azul/async/*.hpp")

add_library (azul_async INTERFACE)
target_sources(azul_async INTERFACE ${SOURCES})
target_link_libraries(azul_async INTERFACE ${PLATFORM_DEPENDENCIES})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/azul/async DESTINATION include/azul/)

if (MSVC)
add_custom_target(azul_async_headers SOURCES ${SOURCES})
endif()
