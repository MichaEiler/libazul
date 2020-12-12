file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_azul_ipc ${TESTS_SOURCES})
target_include_directories (tests_azul_ipc PRIVATE "./")
target_link_libraries(tests_azul_ipc PUBLIC gtest gmock_main azul_ipc)
