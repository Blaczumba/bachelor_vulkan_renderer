#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/ref.h"
#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
class ReferenceCounterWithMetadata : public ReferenceCounter<Resource> {
  void incrementRefCount(HandleFor<Resource> handle) override;

  void decrementRefCount(HandleFor<Resource> handle) override;

public:
  ReferenceCounterWithMetadata();

  Ref<Resource> transferResource(Resource&& resource, const MetadataFor<Resource>& metadata);

protected:
  struct Entry {
    Resource resource;
    MetadataFor<Resource> metadata;
  };

private:
  lib::SparseMap<Entry, MAX_NUMBER_OF<Resource>> _resourceMap;
  std::array<std::atomic<uint64_t>, MAX_NUMBER_OF<Resource>> _refCounts;
  std::vector<HandleFor<Resource>> _freeHandles;  // TODO: Change to std::inplace_vector.

  mutable std::mutex _mutex;
};
