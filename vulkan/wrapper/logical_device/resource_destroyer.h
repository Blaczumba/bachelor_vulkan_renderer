#pragma once

#include <vulkan/vulkan.h>
#include <functional>

#include "lib/thread/worker.h"

using ResourceDestroyerJob = std::function<void(VkDevice, VkAllocationCallbacks*)>;

class ResourceDestroyer {
public:
  virtual ~ResourceDestroyer() = default;

  virtual void destroyResource(ResourceDestroyerJob&& destroyResource) = 0;

  virtual void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks) = 0;
};

class ThreadedResourceDestroyer : public ResourceDestroyer {
public:
  ThreadedResourceDestroyer() = default;

  ~ThreadedResourceDestroyer() override = default;

  void destroyResource(ResourceDestroyerJob&& destroyResource) override;

  void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks) override;

private:
  lib::thread::Worker<VkDevice, VkAllocationCallbacks*> _worker;
};

class ImmediateResourceDestroyer : public ResourceDestroyer {
public:
  ImmediateResourceDestroyer() = default;

  ~ImmediateResourceDestroyer() override = default;

  void destroyResource(ResourceDestroyerJob&& destroyResource) override;

  void setupContext(VkDevice device, VkAllocationCallbacks* allocationCallbacks) override;

private:
  VkDevice _device = VK_NULL_HANDLE;
  VkAllocationCallbacks* _allocationCallback = nullptr;
};
