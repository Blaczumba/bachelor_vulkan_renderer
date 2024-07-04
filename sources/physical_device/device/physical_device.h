#pragma once

#include "instance/instance.h"
#include "surface/surface.h"
#include "physical_device/property_manager/property_manager.h"

#include <vector>
#include <memory>
#include <optional>

class PhysicalDevice {
    VkPhysicalDevice _device;
	std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;

    PhysicalDevicePropertyManager _propertyManager;
public:
	PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);

    VkPhysicalDevice getVkPhysicalDevice() const;
    QueueFamilyIndices getQueueFamilyIndices() const;
    SwapChainSupportDetails getSwapChainSupportDetails() const;
    VkSampleCountFlagBits getMaxMsaaSampleCount() const;
    bool checkTextureFormatSupport(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    bool checkBlittingSupport(VkFormat format) const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    float getMaxSamplerAnisotropy() const;
private:
    VkSampleCountFlagBits _msaaSampleCount;
    QueueFamilyIndices _queueFamilyIndices;
    float _maxSamplerAnisotropy;
};
