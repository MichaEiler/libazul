file (GLOB TESTS_SOURCES "./*.cpp")

add_executable(tests_impulso_async ${TESTS_SOURCES})
target_include_directories (tests_impulso_async PRIVATE "./" "../../include/")
target_link_libraries(tests_impulso_async PUBLIC gtest gmock_main impulso_async)
