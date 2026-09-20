enable_testing()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.oes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(util_test filesystem_test.cc)

target_link_libraries(util_test PRIVATE
    orion
    GTest::gtest_main
)
gtest_discover_tests(util_test)
