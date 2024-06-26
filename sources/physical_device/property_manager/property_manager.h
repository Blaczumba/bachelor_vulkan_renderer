#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevicePropertyManager {
public:
    VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) const;
    float getMaxSamplerAnisotropy(VkPhysicalDevice physicalDevice) const;

    QueueFamilyIndices findQueueFamilyIncides(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;
    SwapChainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device) const;
    std::vector<VkExtensionProperties> getAvailableExtensionProperties(VkPhysicalDevice device) const;

    bool checkDiscreteGPU(VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
};

