#pragma once

#include <vulkan/vulkan.h>
#include <functional>

#include "lib/thread/worker.h"

class ResourceDestroyer {
public:
  virtual ~ResourceDestroyer() = default;

  virtual void destroyResource(std::function<void(VkDevice)>&& destroyResource) = 0;

  virtual void setupContext(VkDevice device) = 0;
};

class ThreadedResourceDestroyer : public ResourceDestroyer {
public:
  ThreadedResourceDestroyer() = default;

  ~ThreadedResourceDestroyer() override = default;

  void destroyResource(std::function<void(VkDevice)>&& destroyResource) override;

  void setupContext(VkDevice device) override;

private:
  lib::thread::Worker<VkDevice> _worker;
};

class ImmediateResourceDestroyer : public ResourceDestroyer {
public:
  ImmediateResourceDestroyer() = default;

  ~ImmediateResourceDestroyer() override = default;

  void destroyResource(std::function<void(VkDevice)>&& destroyResource) override;

  void setupContext(VkDevice device) override;

private:
  VkDevice _device = VK_NULL_HANDLE;
};
