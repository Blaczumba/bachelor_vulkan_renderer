#include "vulkan/resource_manager/reference_counter_with_hashing.h"

#include <format>
#include <mutex>
#include <numeric>

#include "common/util/engine_exception.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/ref.h"

template <typename Resource>
ReferenceCounterWithHashing<Resource>::ReferenceCounterWithHashing()
  : _freeHandles(MAX_NUMBER_OF<Resource>) {
  std::iota(_freeHandles.rbegin(), _freeHandles.rend(), HandleFor<Resource>(0));
}

template <typename Resource>
void ReferenceCounterWithHashing<Resource>::incrementRefCount(HandleFor<Resource> handle) {
  std::shared_lock lock(_mutex);
  _resourceMap.getValue(*handle).refCount.fetch_add(1, std::memory_order_relaxed);
}

template <typename Resource>
void ReferenceCounterWithHashing<Resource>::decrementRefCount(HandleFor<Resource> handle) {
  std::unique_lock lock(_mutex);
  Entry& entry = _resourceMap.getValue(*handle);
  if (entry.refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
    _collisionMap.erase(*entry.metadata);
    _resourceMap.eraseUnsafe(*handle);
    _freeHandles.push_back(handle);
  }
}
