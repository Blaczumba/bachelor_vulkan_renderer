#include "physical_device.h"
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
        QueueFamilyIndices indices = _propertyManager.findQueueFamilyIncides(device, surf);
        bool extensionsSupported = _propertyManager.checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport;
        if (extensionsSupported) {
            swapChainSupport = _propertyManager.querySwapchainSupportDetails(device, surf);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool discreteGPU = _propertyManager.checkDiscreteGPU(device);

        std::array<bool, 4> conditions = {
            indices.isComplete(),
            extensionsSupported,
            swapChainAdequate,
            supportedFeatures.samplerAnisotropy,
            // discreteGPU
        };

        bool suitable = std::all_of(conditions.cbegin(), conditions.cend(), [](bool condition) { return condition; });

        if (suitable) {
            _device = device;
            _msaaSampleCount = _propertyManager.getMaxUsableSampleCount(device);
            _maxSamplerAnisotropy = _propertyManager.getMaxSamplerAnisotropy(device);
            _queueFamilyIndices = indices;
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

QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices() const {
    return _queueFamilyIndices;
}

SwapChainSupportDetails PhysicalDevice::getSwapChainSupportDetails() const {
    return _propertyManager.querySwapchainSupportDetails(_device, _surface->getVkSurface());
}

VkSampleCountFlagBits PhysicalDevice::getMaxMsaaSampleCount() const {
    return _msaaSampleCount;
}

bool PhysicalDevice::checkTextureFormatSupport(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(_device, format, &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
        return true;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
        return true;
    }

    return false;
}

bool PhysicalDevice::checkBlittingSupport(VkFormat format) const {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(_device, format, &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        std::cout << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        return false;
    }
    return true;
}

uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

float PhysicalDevice::getMaxSamplerAnisotropy() const {
    return _maxSamplerAnisotropy;
}