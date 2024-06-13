#pragma once

#include "instance/instance.h"
#include "surface/surface.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <optional>

class PhysicalDevice {
    VkPhysicalDevice _device;
	std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;
public:
	PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);

    VkPhysicalDevice getVkPhysicalDevice();
    VkSampleCountFlagBits getMaxUsableSampleCount();

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

    QueueFamilyIndices getQueueFamilyIncides() const;
    SwapChainSupportDetails getSwapChainSupportDetails() const;
private:
    QueueFamilyIndices queueIndices;
    SwapChainSupportDetails swapChainDetails;

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device) const;
    std::vector<VkExtensionProperties> getAvailableExtensionProperties(VkPhysicalDevice device) const;
};
