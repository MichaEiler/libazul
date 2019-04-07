file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_azul_compute ${TESTS_SOURCES})
target_include_directories (tests_azul_compute PRIVATE "./")
target_link_libraries(tests_azul_compute PUBLIC gtest gmock_main azul_compute)