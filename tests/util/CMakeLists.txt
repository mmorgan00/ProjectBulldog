enable_testing()

# This gives us a test file to run against that mocks the location of resource files in the main application
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.oes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(util_test filesystem_test.cc)

target_link_libraries(util_test PRIVATE
    orion
    GTest::gtest_main
)
gtest_discover_tests(util_test)
