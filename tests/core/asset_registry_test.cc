#include "orion/core/asset_registry.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

TEST(AssetRegistry, InsertReturnsValidHandle) {
  AssetRegistry<std::string> registry;
  auto handle = registry.insert("test");
  auto* result = registry.get(handle);
  EXPECT_NE(result, nullptr);
};
TEST(AssetRegistry, GetReturnsInsertedValue) {
  AssetRegistry<std::string> registry;
  auto handle = registry.insert("test-value");
  auto* result = registry.get(handle);
  EXPECT_EQ(*result, "test-value");
};

TEST(AssetRegistry, MultipleInsertsReturnDistinctHandles) {
  const uint32_t val1 = 42;
  const uint32_t val2 = 5000;
  const uint32_t val3 = 3141592657;
  AssetRegistry<uint32_t> registry;
  auto handle1 = registry.insert(val1);
  auto handle2 = registry.insert(val2);
  auto handle3 = registry.insert(val3);
  EXPECT_NE(handle1, handle2);
  EXPECT_NE(handle1, handle3);
  EXPECT_NE(handle2, handle3);
};

// // Removal.
TEST(AssetRegistry, GetAfterRemoveReturnsNull) {
  AssetRegistry<std::string> registry;
  auto handle = registry.insert("to-be-removed-value");
  registry.remove(handle);
  auto* result = registry.get(handle);
  EXPECT_EQ(result, nullptr);
};
TEST(AssetRegistry, RemoveTwiceIsSafe) {
  AssetRegistry<std::string> registry;
  auto handle = registry.insert("double-remove");
  registry.remove(handle);
  auto* result1 = registry.get(handle);
  EXPECT_EQ(result1, nullptr);
  registry.remove(handle);
  auto* result2 = registry.get(handle);
  EXPECT_EQ(result2, nullptr);
};

TEST(AssetRegistry, StaleHandleAfterSlotReuseReturnsNull) {
  AssetRegistry<std::string> registry;
  auto handle1 = registry.insert("lorem");
  registry.remove(handle1);
  auto handle2 = registry.insert("Ipsum");            // should reuse slot 0
  EXPECT_EQ(handle1.index, handle2.index);            // same slot
  EXPECT_NE(handle1.generation, handle2.generation);  // different gen
  EXPECT_EQ(registry.get(handle1), nullptr);          // old handle invalid
  EXPECT_NE(registry.get(handle2), nullptr);          // new handle valid
}

// // Edge cases.
TEST(AssetRegistry, GetWithOutOfBoundsIndexReturnsNull) {
  AssetRegistry<std::string> registry;
  auto handle1 = registry.insert("lorem");
  const uint32_t oob_index = 50;
  Handle<std::string> invalid_handle =
      Handle<std::string>{.index = oob_index, .generation = 1};
  EXPECT_EQ(registry.get(invalid_handle), nullptr);
}

// // Free list behavior.
TEST(AssetRegistry, FreedSlotsAreReusedBeforeGrowing) {
  AssetRegistry<std::string> registry;
  auto handle1 = registry.insert("Adam");
  auto handle2 = registry.insert("Jack");
  registry.remove(handle1);
  auto handle3 = registry.insert("Ryan");
  EXPECT_EQ(handle3.index, handle1.index);  // reused, not appended
}
