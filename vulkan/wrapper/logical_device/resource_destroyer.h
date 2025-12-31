#pragma once

#include <functional>
#include <vulkan/vulkan.h>

#include "lib/thread/worker.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"

struct DestroyerContext {
  VkDevice device = VK_NULL_HANDLE;
  VkAllocationCallbacks* allocationCallbacks = nullptr;
  MemoryAllocator* memoryAllocator = nullptr;
};

using ResourceDestroyerJob = std::move_only_function<void(DestroyerContext)>;

class ResourceDestroyer {
public:
  virtual ~ResourceDestroyer() = default;

  virtual void destroyResource(ResourceDestroyerJob destroyResource) = 0;

  virtual void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks,
                            MemoryAllocator* memoryAllocator) = 0;
};

using ResourceDestroyerPtr = std::unique_ptr<ResourceDestroyer>;

class ThreadedResourceDestroyer : public ResourceDestroyer {
public:
  ThreadedResourceDestroyer() noexcept = default;

  ~ThreadedResourceDestroyer() override = default;

  void destroyResource(ResourceDestroyerJob destroyResource) override;

  void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks,
                    MemoryAllocator* memoryAllocator) override;

private:
  lib::thread::Worker<16, DestroyerContext> _worker;
};

class ImmediateResourceDestroyer : public ResourceDestroyer {
public:
  ImmediateResourceDestroyer() noexcept = default;

  ~ImmediateResourceDestroyer() override = default;

  void destroyResource(ResourceDestroyerJob destroyResource) override;

  void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks,
                    MemoryAllocator* memoryAllocator) override;

private:
  DestroyerContext _context{};
};
