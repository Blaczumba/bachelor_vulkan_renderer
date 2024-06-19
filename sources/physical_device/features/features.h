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

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
QueueFamilyIndices findQueueFamilyIncides(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
SwapChainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device);
std::vector<VkExtensionProperties> getAvailableExtensionProperties(VkPhysicalDevice device);

bool checkDiscreteGPU(VkPhysicalDevice device);

bool checkDeviceExtensionSupport(VkPhysicalDevice device);
