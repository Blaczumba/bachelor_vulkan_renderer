#include "vulkan/resource_manager/reference_counter_with_metadata.h"

#include <format>
#include <numeric>

#include "common/util/engine_exception.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/ref.h"

template <typename Resource>
ReferenceCounterWithMetadata<Resource>::ReferenceCounterWithMetadata()
  : _freeHandles(MAX_NUMBER_OF<Resource>) {
  std::iota(_freeHandles.rbegin(), _freeHandles.rend(), HandleFor<Resource>(0));
}

template <typename Resource>
Ref<Resource> ReferenceCounterWithMetadata<Resource>::transferResource(
    Resource&& resource, const MetadataFor<Resource>& metadata) {
  HandleFor<Resource> handle;
  {
    std::lock_guard lock(_mutex);
    if (_freeHandles.empty()) [[unlikely]] {
      throw EngineException(
          std::format("No free handles available for {} transfer.", Handle<Resource>::name));
    }
    handle = _freeHandles.back();
    _freeHandles.pop_back();
    _resourceMap.insertUnsafe(*handle, Entry{std::move(resource), metadata});
  }
  _refCounts[*handle].store(1, std::memory_order_relaxed);
  return Ref<Resource>(*this, handle);
}

template <typename Resource>
void ReferenceCounterWithMetadata<Resource>::incrementRefCount(HandleFor<Resource> handle) {
  _refCounts[*handle].fetch_add(1, std::memory_order_relaxed);
}

template <typename Resource>
void ReferenceCounterWithMetadata<Resource>::decrementRefCount(HandleFor<Resource> handle) {
  if (_refCounts[*handle].fetch_sub(1, std::memory_order_release) != 1) [[likely]] {
    return;
  }

  std::atomic_thread_fence(std::memory_order_acquire);

  Resource objectToBeDestroyed;  // Destroyed after the lock is released.
  {
    std::lock_guard lock(_mutex);
    objectToBeDestroyed = std::move(_resourceMap.getValue(*handle).resource);
    _resourceMap.eraseUnsafe(*handle);
    _freeHandles.push_back(handle);
  }
}
