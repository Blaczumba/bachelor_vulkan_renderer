#include "physical_device.h"

#include <algorithm>
#include <array>
#include <stdexcept>

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

PhysicalDevice::PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface)
	: _instance(instance), _surface(surface) {

    std::vector<VkPhysicalDevice> devices = _instance->getAvailablePhysicalDevices();

    for (const auto& device : devices) {
        _device = device;
        if (isDeviceSuitable()) {
            _device = device;
            _msaaSamples = getMaxUsableSampleCount();
            break;
        }
        else {
            _device = nullptr;
        }
    }

    if (_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool PhysicalDevice::isDeviceSuitable() {
    QueueFamilyIndices indices = findQueueFamilies();

    bool extensionsSupported = checkDeviceExtensionSupport();

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(_device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties() const {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

std::vector<VkExtensionProperties> PhysicalDevice::getAvailableExtensionProperties() const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
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

    auto sample = std::find_if(samples.cbegin(), samples.cend(), [&](VkSampleCountFlagBits sample) { return sample & counts; });
    if (sample != samples.cend())
        return *sample;
    throw std::runtime_error("failed to select MSAA sample");
}

bool PhysicalDevice::checkDeviceExtensionSupport() const {
    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensionProperties();

    // Check if all deviceExtensions are in availableExtensions.
    return std::all_of(deviceExtensions.cbegin(), deviceExtensions.cend(), [&](const char* extension) {
        return std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties) {
            return std::strcmp(properties.extensionName, extension) == 0;
        }) != availableExtensions.cend();
    });
}

// !!!!
PhysicalDevice::QueueFamilyIndices PhysicalDevice::findQueueFamilies() {
    PhysicalDevice::QueueFamilyIndices indices;

    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, _surface->getVkSurface(), &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

// !!!!
PhysicalDevice::SwapChainSupportDetails PhysicalDevice::querySwapChainSupport() {
    SwapChainSupportDetails details;

    VkSurfaceKHR surface = _surface->getVkSurface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkPhysicalDevice PhysicalDevice::getVkPhysicalDevice() {
    return _device;
}
