#ifndef TESTS_UTIL_FILESYSTEM_TEST_CC_
#define TESTS_UTIL_FILESYSTEM_TEST_CC_

#include "orion/util/filesystem.h"

#include <gtest/gtest.h>

// test.oes is a reliably available demo scene file populated by the build
// system
TEST(Filesystem, ConfigFilesCanBeFound) {
  File test_file;
  EXPECT_EQ(File::resource("test.oes", &test_file), true);
}
#endif  // TESTS_UTIL_FILESYSTEM_TEST_CC_
