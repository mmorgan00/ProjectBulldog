add_executable(asset_registry_test asset_registry_test.cc)
target_link_libraries(asset_registry_test PRIVATE
    orion
    GTest::gtest_main
)
gtest_discover_tests(asset_registry_test)
