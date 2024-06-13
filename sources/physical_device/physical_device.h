#pragma once

#include "instance/instance.h"
#include "surface/surface.h"
#include "features.h"

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
    QueueFamilyIndices getQueueFamilyIndices() const;
    SwapChainSupportDetails getSwapChainDetails() const;
    VkSampleCountFlagBits getMsaaSampleCount() const;
private:
    QueueFamilyIndices _queueIndices;
    SwapChainSupportDetails _swapChainDetails;
    VkSampleCountFlagBits _msaaSampleCount;
};
