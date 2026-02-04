#pragma once

#include <memory>

#include "common/window/window.h"
#include "vulkan/vulkan.h"
#include "vulkan/wrapper/instance/instance.h"

class Surface {
  Surface(VkSurfaceKHR surface, const Instance& instance) noexcept;

public:
  Surface() = default;

  static Surface create(const Instance& instance, const Window& window);

  Surface(Surface&& other) noexcept;

  Surface& operator=(Surface&& other) noexcept;

  ~Surface();

  VkSurfaceKHR getVkSurface() const noexcept;

private:
  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  const Instance* _instance = nullptr;
};
