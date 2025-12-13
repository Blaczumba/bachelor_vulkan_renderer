#include "resource_destroyer.h"

void ThreadedResourceDestroyer::destroyResource(ResourceDestroyerJob&& destroyResource) {
  _worker.addJob(std::move(destroyResource));
}

void ThreadedResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks, MemoryAllocator* memoryAllocator) {
  _worker.startWorkingThread(DestroyerContext{device, allocationCallbacks, memoryAllocator});
}

void ImmediateResourceDestroyer::destroyResource(ResourceDestroyerJob&& destroyResource) {
  destroyResource(_context);
}

void ImmediateResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks, MemoryAllocator* memoryAllocator) {
  _context = DestroyerContext{device, allocationCallbacks, memoryAllocator};
}
