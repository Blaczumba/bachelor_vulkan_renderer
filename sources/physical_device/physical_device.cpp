#include "physical_device.h"
#include "logical_device/logical_device.h"
#include "config/config.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <iostream>

PhysicalDevice::PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface)
	: _instance(instance), _surface(surface), _device(VK_NULL_HANDLE) {

    std::vector<VkPhysicalDevice> devices = _instance->getAvailablePhysicalDevices();
    VkSurfaceKHR surf = surface->getVkSurface();

    for (const auto device : devices) {
        _propertyManager.initiate(device, surf);
        const QueueFamilyIndices& indices = _propertyManager.getQueueFamilyIndices();
        bool extensionsSupported = _propertyManager.checkDeviceExtensionSupport();

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            const SwapChainSupportDetails swapChainSupport = _propertyManager.getSwapChainSupportDetails();
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool discreteGPU = _propertyManager.checkDiscreteGPU();

        std::array<bool, 5> conditions = {
            indices.isComplete(),
            extensionsSupported,
            swapChainAdequate,
            supportedFeatures.samplerAnisotropy,
            discreteGPU
        };

        bool suitable = std::all_of(conditions.cbegin(), conditions.cend(), [](bool condition) { return condition; });

        if (suitable) {
            _device = device;
            break;
        }
    }

    if (_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

VkPhysicalDevice PhysicalDevice::getVkPhysicalDevice() const {
    return _device;
}

const PhysicalDevicePropertyManager& PhysicalDevice::getPropertyManager() const {
    return _propertyManager;
}

std::shared_ptr<LogicalDevice> PhysicalDevice::createLogicalDevice() {
    return std::make_shared<LogicalDevice>(*this);
}