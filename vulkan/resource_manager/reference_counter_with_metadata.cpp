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
  std::unique_lock lock(_mutex);
  if (_freeHandles.empty()) [[unlikely]] {
    throw EngineException(
        std::format("No free handles available for {} transfer.", Handle<Resource>::name));
  }
  handle = _freeHandles.back();
  _freeHandles.pop_back();
  _resourceMap.insertUnsafe(*handle, Entry{std::move(resource), 1});

  return Ref<Resource>(*this, handle);
}

template <typename Resource>
void ReferenceCounterWithMetadata<Resource>::incrementRefCount(HandleFor<Resource> handle) {
  std::shared_lock lock(_mutex);
  _resourceMap.getValue(*handle).refCount.fetch_add(1, std::memory_order_relaxed);
}

template <typename Resource>
void ReferenceCounterWithMetadata<Resource>::decrementRefCount(HandleFor<Resource> handle) {
  std::unique_lock lock(_mutex);
  if (_resourceMap.getValue(*handle).refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
    _resourceMap.erase(*handle);
    _freeHandles.push_back(handle);
  }
}
