#pragma once

#include <atomic>
#include <shared_mutex>

#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
class ReferenceCounterWithHashing : public ReferenceCounter<Resource> {
  void incrementRefCount(HandleFor<Resource> handle) override;

  void decrementRefCount(HandleFor<Resource> handle) override;

public:
  ReferenceCounterWithHashing();

protected:
  struct Entry {
    Resource resource;
    const MetadataFor<Resource>* metadata;
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

protected:
  lib::SparseMap<Entry, MAX_NUMBER_OF<Resource>> _resourceMap;
  std::vector<HandleFor<Resource>> _freeHandles;  // TODO: Change to std::inplace_vector.

  std::unordered_map<MetadataFor<Resource>, HandleFor<Resource>, HasherFor<Resource>> _collisionMap;

  mutable std::shared_mutex _mutex;
};
