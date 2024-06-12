#pragma once

#include "instance/instance.h"
#include "surface/surface.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <optional>

class PhysicalDevice {
	std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;
    VkPhysicalDevice _device;
    VkSampleCountFlagBits _msaaSamples; // Remove it
public:
	PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);
    bool isDeviceSuitable();

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;
    std::vector<VkExtensionProperties> getAvailableExtensionProperties() const;
    VkSampleCountFlagBits getMaxUsableSampleCount();

    bool checkDeviceExtensionSupport() const;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    QueueFamilyIndices findQueueFamilies();
    SwapChainSupportDetails querySwapChainSupport();

    VkPhysicalDevice getVkPhysicalDevice();
};
