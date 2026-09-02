#ifndef ASSET_REGISTRY_H_
#define ASSET_REGISTRY_H_
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "resource.h"

template <typename T>
class AssetRegistry {
 public:
  Resource<T> insert(T value) {
    if (!free_list_.empty()) {
      uint32_t idx = free_list_.back();
      free_list_.pop_back();
      slots_[idx].data = std::move(value);
      // generation was already bumped when this slot was freed
      return Resource<T>{idx, slots_[idx].generation};
    }
    slots_.push_back(Slot{1, std::move(value)});
    return Resource<T>{static_cast<uint32_t>(slots_.size() - 1), 1};
  }

  T* get(Resource<T> handle) {
    if (handle.index >= slots_.size()) {
      return nullptr;
    }
    Slot& slot = slots_[handle.index];
    if (slot.generation != handle.generation) {
      return nullptr;
    }
    if (!slot.data.has_value()) {
      return nullptr;
    }
    return &*slot.data;
  }

  const T* get(Resource<T> handle) const {
    if (handle.index >= slots_.size()) {
      return nullptr;
    }
    const Slot& slot = slots_[handle.index];
    if (slot.generation != handle.generation) {
      return nullptr;
    }
    if (!slot.data.has_value()) {
      return nullptr;
    }
    return &*slot.data;
  }

  bool remove(Resource<T> handle) {
    if (get(handle) == nullptr) {
      return false;
    }
    slots_[handle.index].data.reset();
    slots_[handle.index].generation++;
    free_list_.push_back(handle.index);
    return true;
  }

  size_t size() const { return slots_.size() - free_list_.size(); }

 private:
  struct Slot {
    uint32_t generation = 0;
    std::optional<T> data;
  };

  std::vector<Slot> slots_;
  std::vector<uint32_t> free_list_;
};
;

#endif  // ASSET_REGISTRY_H_
