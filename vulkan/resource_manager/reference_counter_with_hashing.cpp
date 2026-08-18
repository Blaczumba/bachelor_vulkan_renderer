#include "vulkan/resource_manager/reference_counter_with_hashing.h"

#include <mutex>
#include <numeric>

#include "vulkan/resource_manager/handle.h"

template <typename Resource>
ReferenceCounterWithHashing<Resource>::ReferenceCounterWithHashing()
  : _freeHandles(MAX_NUMBER_OF<Resource>) {
  std::iota(_freeHandles.rbegin(), _freeHandles.rend(), HandleFor<Resource>(0));
}

template <typename Resource>
void ReferenceCounterWithHashing<Resource>::incrementRefCount(HandleFor<Resource> handle) {
  _refCounts[*handle].fetch_add(1, std::memory_order_relaxed);
}

template <typename Resource>
void ReferenceCounterWithHashing<Resource>::decrementRefCount(HandleFor<Resource> handle) {
  if (_refCounts[*handle].fetch_sub(1, std::memory_order_release) != 1) [[likely]] {
    return;
  }

  std::atomic_thread_fence(std::memory_order_acquire);

  Resource objectToBeDestroyed;  // Destroyed after the lock is released.
  {
    std::lock_guard lock(_mutex);
    Entry& entry = _resourceMap.getValue(*handle);
    objectToBeDestroyed = std::move(entry.resource);
    _collisionMap.erase(*entry.metadata);
    _resourceMap.eraseUnsafe(*handle);
    _freeHandles.push_back(handle);
  }
}
