#pragma once

#include <atomic>
#include <mutex>

#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
class ReferenceCounterWithHashing : public ReferenceCounter<Resource> {
public:
  ReferenceCounterWithHashing();

protected:
  void incrementRefCount(HandleFor<Resource> handle) override;

  void decrementRefCount(HandleFor<Resource> handle) override;

  struct Entry {
    Resource resource;
    const MetadataFor<Resource>* metadata;
  };

  lib::SparseMap<Entry, MAX_NUMBER_OF<Resource>> _resourceMap;
  std::array<std::atomic<uint64_t>, MAX_NUMBER_OF<Resource>> _refCounts;
  std::vector<HandleFor<Resource>> _freeHandles;  // TODO: Change to std::inplace_vector.

  std::unordered_map<MetadataFor<Resource>, HandleFor<Resource>, HasherFor<Resource>> _collisionMap;

  mutable std::mutex _mutex;
};
