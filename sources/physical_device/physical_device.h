#pragma once

#include "property_manager.h"
#include "window/window/window.h"

#include <memory>
#include <optional>
#include <vector>

class LogicalDevice;

class PhysicalDevice {
    VkPhysicalDevice _device = VK_NULL_HANDLE;
    const Window& _window;

    PhysicalDevicePropertyManager _propertyManager;

public:
	PhysicalDevice(const Window& window);

    const VkPhysicalDevice getVkPhysicalDevice() const;
    const Window& getWindow() const;
    const PhysicalDevicePropertyManager& getPropertyManager() const;

    std::unique_ptr<LogicalDevice> createLogicalDevice();
};
