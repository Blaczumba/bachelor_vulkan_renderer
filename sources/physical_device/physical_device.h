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
	std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;

    PhysicalDevicePropertyManager _propertyManager;

public:
	PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);

    VkPhysicalDevice getVkPhysicalDevice() const;
    const PhysicalDevicePropertyManager& getPropertyManager() const;

    std::shared_ptr<LogicalDevice> createLogicalDevice();

};
