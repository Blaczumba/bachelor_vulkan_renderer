#include "resource_destroyer.h"

void ThreadedResourceDestroyer::destroyResource(std::function<void(VkDevice)>&& destroyResource) {
  _worker.addJob(std::move(destroyResource));
}

void ThreadedResourceDestroyer::setupContext(VkDevice device) {
  _worker.startWorkingThread(device);
}

void ImmediateResourceDestroyer::destroyResource(std::function<void(VkDevice)>&& destroyResource) {
  destroyResource(_device);
}

void ImmediateResourceDestroyer::setupContext(VkDevice device) {
  _device = device;
}
