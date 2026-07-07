#ifndef ORION_CORE_RESOURCE_H_
#define ORION_CORE_RESOURCE_H_
template <typename T>
struct Resource {
  uint32_t index = 0;
  uint32_t generation = 0;

  // Default-constructed handle is invalid (generation 0 is never handed out).
  bool valid() const { return generation != 0; }

  bool operator==(const Resource& other) const {
    return index == other.index && generation == other.generation;
  }
  bool operator!=(const Resource& other) const { return !(*this == other); }
};

#endif  // ORION_CORE_RESOURCE_H_
