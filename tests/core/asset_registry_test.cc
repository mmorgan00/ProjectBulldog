#include "orion/core/asset_registry.h"

#include <gtest/gtest.h>

#include <string>

TEST(AssetRegistry, InsertReturnsValidHandle) {
  AssetRegistry<std::string> registry;
  auto handle = registry.insert("test");
  auto* result = registry.get(handle);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, "test");
};
// TEST(AssetRegistry, GetReturnsInsertedValue){...};

// TEST(AssetRegistry, MultipleInsertsReturnDistinctHandles){...};

// // Removal.
// TEST(AssetRegistry, GetAfterRemoveReturnsNull){...};
// TEST(AssetRegistry, RemoveTwiceIsSafe){...};

// // THE generation tests — these are the whole point of this design.
// TEST(AssetRegistry, StaleHandleAfterRemoveReturnsNull) {
//   AssetRegistry<uint32_t> registry;
//   auto h1 = registry.insert(20);
//   registry.remove(h1);
//   EXPECT_EQ(registry.get(h1), nullptr);
// };

// TEST(AssetRegistry, StaleHandleAfterSlotReuseReturnsNull) {
//   auto h1 = registry.insert(MeshA);
//   registry.remove(h1);
//   auto h2 = registry.insert(MeshB);         // reuses slot 0
//   EXPECT_EQ(h1.index, h2.index);            // same slot
//   EXPECT_NE(h1.generation, h2.generation);  // different gen
//   EXPECT_EQ(registry.get(h1), nullptr);     // old handle invalid
//   EXPECT_NE(registry.get(h2), nullptr);     // new handle valid
// }

// // Edge cases.
// TEST(AssetRegistry, GetWithOutOfBoundsIndexReturnsNull){
//     ...} TEST(AssetRegistry, DefaultConstructedHandleIsInvalid){...}

// // Free list behavior.
// TEST(AssetRegistry, FreedSlotsAreReusedBeforeGrowing) {
//   auto h1 = registry.insert(MeshA);
//   auto h2 = registry.insert(MeshB);
//   registry.remove(h1);
//   auto h3 = registry.insert(MeshC);
//   EXPECT_EQ(h3.index, h1.index);  // reused, not appended
// }
