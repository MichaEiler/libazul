file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_azul_utils ${TESTS_SOURCES})
target_include_directories (tests_azul_utils PRIVATE "./" "../../include/")
target_link_libraries(tests_azul_utils PUBLIC gtest gmock_main azul_utils)
