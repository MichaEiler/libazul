file (GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/azul/utils/*.hpp")

add_library (azul_utils INTERFACE)
target_sources(azul_utils INTERFACE ${SOURCES})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/azul/utils DESTINATION include/azul/)

if (MSVC)
add_custom_target(azul_utils_headers SOURCES ${SOURCES})
endif()
