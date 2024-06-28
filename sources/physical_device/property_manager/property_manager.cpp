#include "property_manager.h"
#include "config/config.h"

#include <cstring>
#include <algorithm>
#include <array>
#include <stdexcept>

QueueFamilyIndices PhysicalDevicePropertyManager::findQueueFamilyIncides(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
    QueueFamilyIndices indices;

    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties(physicalDevice);

    for (uint32_t i = 0; i < queueFamilies.size() && !indices.isComplete(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (presentSupport) {
            indices.presentFamily = i;
        }
    }

    return indices;
}

VkSampleCountFlagBits PhysicalDevicePropertyManager::getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) const {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

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

float PhysicalDevicePropertyManager::getMaxSamplerAnisotropy(VkPhysicalDevice physicalDevice) const {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    return properties.limits.maxSamplerAnisotropy;
}

SwapChainSupportDetails PhysicalDevicePropertyManager::querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


std::vector<VkQueueFamilyProperties> PhysicalDevicePropertyManager::getQueueFamilyProperties(VkPhysicalDevice device) const {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

std::vector<VkExtensionProperties> PhysicalDevicePropertyManager::getAvailableExtensionProperties(VkPhysicalDevice device) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

bool PhysicalDevicePropertyManager::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensionProperties(device);

    // Check if all deviceExtensions are in availableExtensions.
    return std::all_of(deviceExtensions.cbegin(), deviceExtensions.cend(), [&](const char* extension) {
        return std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties) {
            return std::strcmp(properties.extensionName, extension) == 0;
            }) != availableExtensions.cend();
        });
}

bool PhysicalDevicePropertyManager::checkDiscreteGPU(VkPhysicalDevice device) const {
    VkPhysicalDeviceProperties properties;

    vkGetPhysicalDeviceProperties(device, &properties);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}
