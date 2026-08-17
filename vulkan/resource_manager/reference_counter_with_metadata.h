#pragma once

#include <atomic>
#include <cstdint>
#include <shared_mutex>

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
    std::atomic<uint32_t> refCount = 0;

    Entry() = default;

    Entry(Entry&& other) noexcept
      : resource(std::move(other.resource)), metadata(other.metadata),
        refCount(other.refCount.load(std::memory_order_relaxed)) {}

    Entry& operator=(Entry&& other) noexcept {
      if (this == &other) {
        return *this;
      }
      resource = std::move(other.resource);
      metadata = other.metadata;
      refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
      return *this;
    }
  };

private:
  lib::SparseMap<Entry, MAX_NUMBER_OF<Resource>> _resourceMap;
  std::vector<HandleFor<Resource>> _freeHandles;  // TODO: Change to std::inplace_vector.

  std::shared_mutex _mutex;
};
