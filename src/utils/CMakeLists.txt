file (GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/impulso/utils/*.hpp")

add_library (impulso_utils INTERFACE)
target_sources(impulso_utils INTERFACE ${SOURCES})

if (MSVC)
add_custom_target(impulso_utils_headers SOURCES ${SOURCES})
endif()