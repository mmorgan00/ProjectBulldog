#include "orion/util/filesystem.h"

#include <gtest/gtest.h>

// test.oes is a reliably available demo scene file populated by the build
// system
TEST(Filesystem, ResourceFilesCanBeFound) {
  File test_file;
  EXPECT_EQ(File::resource("test.oes", &test_file), true);
}

TEST(Filesystem, NonexistantFiles) {
  File test_file;
  EXPECT_NE(File::resource("not-existing-test.oes", &test_file), true);
}
