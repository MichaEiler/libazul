file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_azul_async ${TESTS_SOURCES})
target_include_directories (tests_azul_async PRIVATE "./" "../../include/")
target_link_libraries(tests_azul_async PUBLIC gtest gmock_main azul_async)
