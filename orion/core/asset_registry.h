#ifndef ASSET_REGISTRY_H_
#define ASSET_REGISTRY_H_

template <typename T>
struct Handle {
  uint32_t index;       // slot in registry's storage
  uint32_t generation;  // bumped when slot is reused
};

/**
 * @brief Registry for a specific type of resource (Mesh, Audio, Material)
 * @detail The registry plays relatively dumb on concurrency and
 * synchronizaiton. The registry does *NOT* handle loading,
 * which is where the parallelization heavy work will likely go, but is not
 * a concern of the Registry
 **/
template <typename T>
class AssetRegistry {
  struct Slot {
    uint32_t generation;
    std::optional<T> data;
  };

  std::vector<Slot> slots;
  std::vector<uint32_t> free_list;

 public:
  T* get(Handle<T> handle);
  Handle<T> insert(T value);
  void remove(Handle<T> handle);
};

#endif  // ASSET_REGISTRY_H_
