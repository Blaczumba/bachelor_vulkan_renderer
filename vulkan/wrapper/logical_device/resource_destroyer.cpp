#include "resource_destroyer.h"

#include <functional>
#include <vulkan/vulkan.h>

#include "lib/thread/worker.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"

void ThreadedResourceDestroyer::destroyResource(Job destroyResource) {
  _worker.addJob(std::move(destroyResource));
}

void ThreadedResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks, MemoryAllocator* memoryAllocator) {
  _worker.startWorkingThread(DestroyerContext{device, allocationCallbacks, memoryAllocator});
}

void ImmediateResourceDestroyer::destroyResource(Job destroyResource) {
  destroyResource(_context);
}

void ImmediateResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks, MemoryAllocator* memoryAllocator) {
  _context = DestroyerContext{device, allocationCallbacks, memoryAllocator};
}
