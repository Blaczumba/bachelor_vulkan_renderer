#include "physical_device.h"
#include "config/config.h"

#include <algorithm>
#include <array>
#include <stdexcept>

PhysicalDevice::PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface)
	: _instance(instance), _surface(surface), _device(VK_NULL_HANDLE) {

    std::vector<VkPhysicalDevice> devices = _instance->getAvailablePhysicalDevices();
    VkSurfaceKHR surf = surface->getVkSurface();

    for (const auto& device : devices) {
        QueueFamilyIndices indices = findQueueFamilyIncides(device, surf);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport;
        if (extensionsSupported) {
            swapChainSupport = querySwapchainSupportDetails(device, surf);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool suitable = indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;

        if (suitable) {
            _device = device;
            _msaaSampleCount = getMaxUsableSampleCount(device);
            _queueIndices = indices;
            _swapChainDetails = swapChainSupport;
            break;
        }
    }

    if (_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

VkPhysicalDevice PhysicalDevice::getVkPhysicalDevice() {
    return _device;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices() const {
    return _queueIndices;
}

SwapChainSupportDetails PhysicalDevice::getSwapChainDetails() const {
    return _swapChainDetails;
}

VkSampleCountFlagBits PhysicalDevice::getMsaaSampleCount() const {
    return _msaaSampleCount;
}
