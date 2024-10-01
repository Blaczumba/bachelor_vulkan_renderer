#include "property_manager.h"
#include "config/config.h"

#include <cstring>
#include <algorithm>
#include <array>
#include <stdexcept>

void PhysicalDevicePropertyManager::initiate(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface) {
    _physicalDevice = physicalDevice;
    _surface = surface;

    vkGetPhysicalDeviceProperties(_physicalDevice, &_properties);

    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_memoryProperties);

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);

    _availableExtensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, _availableExtensions.data());

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

    _queueFamilies.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, _queueFamilies.data());

    _queueFamilyIndices = findQueueFamilyIncides();
}

QueueFamilyIndices PhysicalDevicePropertyManager::findQueueFamilyIncides() const {
    QueueFamilyIndices indices;

    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size() && !indices.isComplete(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }

        if (presentSupport) {
            indices.presentFamily = i;
        }
    }

    return indices;
}

VkSampleCountFlagBits PhysicalDevicePropertyManager::getMaxUsableSampleCount() const {
    VkSampleCountFlags counts = _properties.limits.framebufferColorSampleCounts & _properties.limits.framebufferDepthSampleCounts;

    // Sorted msaa samples.
    std::array<VkSampleCountFlagBits, 7> samples = {
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT,
        VK_SAMPLE_COUNT_1_BIT
    };

    return *std::find_if(samples.cbegin(), samples.cend(), [&](VkSampleCountFlagBits sample) { return sample & counts; });
}

float PhysicalDevicePropertyManager::getMaxSamplerAnisotropy() const {
    return _properties.limits.maxSamplerAnisotropy;
}

const VkPhysicalDeviceLimits& PhysicalDevicePropertyManager::getPhysicalDeviceLimits() const {
    return _properties.limits;
}

SwapChainSupportDetails PhysicalDevicePropertyManager::querySwapchainSupportDetails() const {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


const QueueFamilyIndices& PhysicalDevicePropertyManager::getQueueFamilyIndices() const {
    return _queueFamilyIndices;
}

SwapChainSupportDetails PhysicalDevicePropertyManager::getSwapChainSupportDetails() const {
    return querySwapchainSupportDetails();
}

const std::vector<VkQueueFamilyProperties>& PhysicalDevicePropertyManager::getQueueFamilyProperties() const {
    return _queueFamilies;
}

const std::vector<VkExtensionProperties>& PhysicalDevicePropertyManager::getAvailableExtensionProperties() const {
    return _availableExtensions;
}

bool PhysicalDevicePropertyManager::checkDeviceExtensionSupport() const {
    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensionProperties();

    // Check if all deviceExtensions are in availableExtensions.
    return std::all_of(deviceExtensions.cbegin(), deviceExtensions.cend(), [&](const char* extension) {
        return std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties) {
            return std::strcmp(properties.extensionName, extension) == 0;
            }) != availableExtensions.cend();
        });
}

bool PhysicalDevicePropertyManager::isDiscreteGPU() const {
    return _properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool PhysicalDevicePropertyManager::checkBlittingSupport(VkFormat format) const {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        return false;
    }
    return true;
}

bool PhysicalDevicePropertyManager::checkTextureFormatSupport(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
        return true;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
        return true;
    }

    return false;
}

uint32_t PhysicalDevicePropertyManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    for (uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
}
