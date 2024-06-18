#pragma once

#include "instance/instance.h"
#include "surface/surface.h"
#include "physical_device/features/features.h"

#include <vector>
#include <memory>
#include <optional>

class PhysicalDevice {
    VkPhysicalDevice _device;
	std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;
public:
	PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);

    VkPhysicalDevice getVkPhysicalDevice() const;
    QueueFamilyIndices getQueueFamilyIndices() const;
    SwapChainSupportDetails getSwapChainSupportDetails() const;
    VkSampleCountFlagBits getMaxMsaaSampleCount() const;
    bool checkTextureFormatSupport(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
private:
    VkSampleCountFlagBits _msaaSampleCount;
    QueueFamilyIndices _queueFamilyIndices;
};
