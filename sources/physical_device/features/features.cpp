#include "features.h"
#include "config/config.h"

#include <algorithm>
#include <array>
#include <stdexcept>

QueueFamilyIndices findQueueFamilyIncides(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
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

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
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

SwapChainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
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


std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

std::vector<VkExtensionProperties> getAvailableExtensionProperties(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensionProperties(device);

    // Check if all deviceExtensions are in availableExtensions.
    return std::all_of(deviceExtensions.cbegin(), deviceExtensions.cend(), [&](const char* extension) {
        return std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties) {
            return std::strcmp(properties.extensionName, extension) == 0;
            }) != availableExtensions.cend();
        });
}

bool checkDiscreteGPU(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;

    vkGetPhysicalDeviceProperties(device, &properties);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}
