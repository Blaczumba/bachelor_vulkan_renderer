#pragma once

#include "instance/instance.h"
#include "surface/surface.h"
#include "property_manager.h"

#include <vector>
#include <memory>
#include <optional>

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

    std::shared_ptr<LogicalDevice> createLogicalDevice();

};
