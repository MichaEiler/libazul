
file (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/clcpp/*.cpp"
                   "${CMAKE_SOURCE_DIR}/include/impulso/compute/clcpp/*.hpp")

add_library (impulso_compute ${SOURCES})
target_include_directories (impulso_compute PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_include_directories (impulso_compute PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(impulso_compute impulso_async)
