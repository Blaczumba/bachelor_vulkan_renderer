#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"

class Instance {
  Instance(VkInstance instance) noexcept;

public:
  Instance() noexcept = default;

  Instance(Instance&& other) noexcept;

  Instance& operator=(Instance&& other) noexcept;

  ~Instance();

  static Instance create(
      std::string_view engineName, std::span<const char* const> requiredExtensions,
      PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

  static Instance wrap(VkInstance instance);

  VkInstance getVkInstance() const noexcept;

  lib::Buffer<VkPhysicalDevice> getAvailablePhysicalDevices() const;

private:
  VkInstance _instance = VK_NULL_HANDLE;
};
