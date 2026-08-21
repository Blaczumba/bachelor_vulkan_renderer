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
public:
  ReferenceCounterWithMetadata();

  Ref<Resource> transferResource(Resource&& resource, const MetadataFor<Resource>& metadata);

  // Must be called when related Ref<Resource> is still alive.
  VulkanObjectFor<Resource> getVkResource(HandleFor<Resource> handle) const;

  // Must be called when related Ref<Resource> is still alive.
  const MetadataFor<Resource>& getMetadata(HandleFor<Resource> handle) const;

  size_t size() const;

protected:
  void incrementRefCount(HandleFor<Resource> handle) override;

  void decrementRefCount(HandleFor<Resource> handle) override;

  struct Entry {
    Resource resource;
    MetadataFor<Resource> metadata;
  };

  std::array<Entry, MAX_NUMBER_OF<Resource>> _entries;
  std::array<std::atomic<uint16_t>, MAX_NUMBER_OF<Resource>> _refCounts;
  std::vector<HandleFor<Resource>> _freeHandles;  // TODO: Change to std::inplace_vector.

  mutable std::mutex _mutex;
};
