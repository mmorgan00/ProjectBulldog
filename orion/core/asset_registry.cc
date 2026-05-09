#include "asset_registry.h"

template <typename T>
T* AssetRegistry<T>::get(Handle<T> handle) {
  if (handle.index >= slots.size()) {
    return nullptr;
  }
  Slot& slot = slots[handle.index];
  if (slot.generation != handle.generation) {
    return nullptr;
  }
  if (!slot.data.has_value()) {
    return nullptr;
  }
  return &*slot.data;
}

/**
 * Uses the back of a vector for constant time operations. Inserts append to the
 * back, so this gets us constant time operations on both fronts with dynamic
 * sizing
 */
template <typename T>
Handle<T> AssetRegistry<T>::insert(T value) {
  if (!free_list.empty()) {
    uint32_t idx = free_list.back();
    free_list.pop_back();
    slots[idx].data = std::move(value);
    // Generation is bumped on a remove, so if there is a free list entry it's
    // already 'configured'
    return {idx, slots[idx].generation};
  }
  slots.push_back({.generation = 1, .data = std::move(value)});
  return Handle<T>{static_cast<uint32_t>(slots.size() - 1), 1};
}

template <typename T>
void AssetRegistry<T>::remove(Handle<T> handle) {
  if (get(handle) == nullptr) {
    return;
  }
  slots[handle.index].data.reset();
  slots[handle.index].generation++;  // invalidates other handles
  free_list.push_back(handle.index);
}
