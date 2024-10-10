#pragma once

#include "instance/instance.h"
#include "property_manager.h"
#include "surface/surface.h"

#include <memory>
#include <optional>
#include <vector>

class LogicalDevice;

class PhysicalDevice {
    VkPhysicalDevice _device;
	const std::shared_ptr<Instance> _instance;
    const std::shared_ptr<Surface> _surface;

    PhysicalDevicePropertyManager _propertyManager;

public:
	PhysicalDevice(const std::shared_ptr<Instance>& instance, const std::shared_ptr<Surface>& surface);

    const VkPhysicalDevice getVkPhysicalDevice() const;
    const PhysicalDevicePropertyManager& getPropertyManager() const;

    std::unique_ptr<LogicalDevice> createLogicalDevice();

};
