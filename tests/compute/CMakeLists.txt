file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_impulso_compute ${TESTS_SOURCES})
target_include_directories (tests_impulso_compute PRIVATE "./")
target_link_libraries(tests_impulso_compute PUBLIC gtest gmock_main impulso_compute)