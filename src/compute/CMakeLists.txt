
file (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/clcpp/*.cpp"
                   "${CMAKE_SOURCE_DIR}/include/azul/compute/clcpp/*.hpp")

add_library (azul_compute ${SOURCES})
target_include_directories (azul_compute PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_include_directories (azul_compute PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(azul_compute azul_async)

install(TARGETS azul_compute DESTINATION lib/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/azul/compute DESTINATION include/azul/)

