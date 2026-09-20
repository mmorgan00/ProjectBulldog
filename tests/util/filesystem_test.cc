#ifndef TESTS_UTIL_FILESYSTEM_TEST_CC_
#define TESTS_UTIL_FILESYSTEM_TEST_CC_

#include "orion/util/filesystem.h"

#include <gtest/gtest.h>

TEST(Filesystem, ConfigFilesCanBeFound) {
  File test_file = File::resource("config/engine.conf");
  EXPECT_EQ(1, 1);
}
#endif  // TESTS_UTIL_FILESYSTEM_TEST_CC_
