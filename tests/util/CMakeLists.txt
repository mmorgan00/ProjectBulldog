enable_testing()

add_executable(util_test filesystem_test.cc)
target_link_libraries(util_test PRIVATE
    orion
    GTest::gtest_main
)
gtest_discover_tests(util_test)
