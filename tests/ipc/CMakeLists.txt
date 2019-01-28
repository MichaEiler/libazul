if (${SUPPORTS_IPC})
    file (GLOB TESTS_SOURCES "./*.cpp")

    add_executable(tests_impulso_ipc ${TESTS_SOURCES})
    target_include_directories (tests_impulso_ipc PRIVATE "./")
    target_link_libraries(tests_impulso_ipc PUBLIC gtest gmock_main impulso_ipc)
endif()
