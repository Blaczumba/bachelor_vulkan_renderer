#include "vulkan/resource_manager/reference_counter_with_hashing.h"

#include <format>
#include <mutex>
#include <numeric>
#include <utility>

#include "common/util/engine_exception.h"
#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/ref.h"

template <typename Resource>
ReferenceCounterWithHashing<Resource>::ReferenceCounterWithHashing()
  : _freeHandles(MAX_NUMBER_OF<Resource>) {
  std::iota(_freeHandles.rbegin(), _freeHandles.rend(), HandleFor<Resource>(0));
}

template <typename Resource>
Ref<Resource> ReferenceCounterWithHashing<Resource>::getOrCreateResource(
    std::function<Resource(const LogicalDevice&, const MetadataFor<Resource>&)>&& creationFunction,
    const LogicalDevice& logicalDevice, const MetadataFor<Resource>& metadata) {
  std::lock_guard lock(_mutex);

  auto [it, inserted] = _collisionMap.try_emplace(metadata);
  if (!inserted) [[likely]] {
    return Ref<Resource>(*this, it->second);
  }

  if (_freeHandles.empty()) [[unlikely]] {
    _collisionMap.erase(it);
    throw EngineException(
        std::format("No free handles available for {} creation.", NAME_OF<Resource>));
  }

  const HandleFor<Resource> handle = _freeHandles.back();

  // Creating the resource under the lock is not ideal, but it is necessary to ensure that the
  // resource is created only once. The other blocked thread instead of wasting time on creating the
  // resource just waits for it to be created.
  Resource created = creationFunction(logicalDevice, metadata);

  _freeHandles.pop_back();
  it->second = handle;
  _entries[*handle] = Entry{std::move(created), &it->first};
  return Ref<Resource>(*this, handle);
}

template <typename Resource>
VulkanObjectFor<Resource> ReferenceCounterWithHashing<Resource>::getVulkanObject(
    HandleFor<Resource> handle) const {
  return _entries[*handle].resource.getVkResource();
}

template <typename Resource>
const MetadataFor<Resource>& ReferenceCounterWithHashing<Resource>::getMetadata(
    HandleFor<Resource> handle) const {
  return *_entries[*handle].metadata;
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

    // Resurrection guard.
    if (_refCounts[*handle].load(std::memory_order_relaxed) != 0) [[unlikely]] {
      return;
    }

    Entry& entry = _entries[*handle];
    objectToBeDestroyed = std::move(entry.resource);
    _collisionMap.erase(*entry.metadata);
    _freeHandles.push_back(handle);
  }
}

template class ReferenceCounterWithHashing<Sampler>;
