# Add utils_tests executable
add_executable(utils_tests
    test_enum_flags.cpp
    test_enum_range.cpp
    test_enum_shift.cpp
)
target_link_libraries(utils_tests
    PRIVATE
        utils
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(utils_tests)
