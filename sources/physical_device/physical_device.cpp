#include "physical_device.h"
#include "config/config.h"

#include <algorithm>
#include <array>
#include <stdexcept>

PhysicalDevice::PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface)
	: _instance(instance), _surface(surface), _device(VK_NULL_HANDLE) {

    std::vector<VkPhysicalDevice> devices = _instance->getAvailablePhysicalDevices();

    for (const auto& device : devices) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport;
        if (extensionsSupported) {
            swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool suitable = indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;

        if (suitable) {
            _device = device;
            queueIndices = indices;
            swapChainDetails = swapChainSupport;
            break;
        }
    }

    if (_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

VkPhysicalDevice PhysicalDevice::getVkPhysicalDevice() {
    return _device;
}

VkSampleCountFlagBits PhysicalDevice::getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_device, &physicalDeviceProperties);

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

PhysicalDevice::QueueFamilyIndices PhysicalDevice::getQueueFamilyIncides() const {
    return queueIndices;
}

PhysicalDevice::SwapChainSupportDetails PhysicalDevice::getSwapChainSupportDetails() const {
    return swapChainDetails;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensionProperties(device);

    // Check if all deviceExtensions are in availableExtensions.
    return std::all_of(deviceExtensions.cbegin(), deviceExtensions.cend(), [&](const char* extension) {
        return std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties) {
            return std::strcmp(properties.extensionName, extension) == 0;
        }) != availableExtensions.cend();
    });
}

PhysicalDevice::QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice device) {
    PhysicalDevice::QueueFamilyIndices indices;

    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties(device);

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface->getVkSurface(), &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

PhysicalDevice::SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    VkSurfaceKHR surface = _surface->getVkSurface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties(VkPhysicalDevice device) const {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

std::vector<VkExtensionProperties> PhysicalDevice::getAvailableExtensionProperties(VkPhysicalDevice device) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

bool PhysicalDevice::QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}
