#include "resource_destroyer.h"

void ThreadedResourceDestroyer::destroyResource(
    ResourceDestroyerJob&& destroyResource) {
  _worker.addJob(std::move(destroyResource));
}

void ThreadedResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks) {
  _worker.startWorkingThread(device, allocationCallbacks);
}

void ImmediateResourceDestroyer::destroyResource(
    ResourceDestroyerJob&& destroyResource) {
  destroyResource(_device, _allocationCallback);
}

void ImmediateResourceDestroyer::setupContext(
    VkDevice device, VkAllocationCallbacks* allocationCallbacks) {
  _device = device;
  _allocationCallback = allocationCallbacks;
}
