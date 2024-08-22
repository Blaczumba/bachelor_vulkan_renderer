#include "swapchain.h"
#include "features/swapchain_utils.h"
#include "physical_device/physical_device.h"
#include "logical_device/logical_device.h"
#include "surface/surface.h"
#include "window/window/window.h"
#include "memory_objects/image.h"

#include <algorithm>
#include <stdexcept>

Swapchain::Swapchain(const Surface& surface, const Window& window, const LogicalDevice& logicalDevice)
	: _surface(surface), _window(window), _logicalDevice(logicalDevice) {
    create();
}

Swapchain::~Swapchain() {
    cleanup();
}

const VkSwapchainKHR Swapchain::getVkSwapchain() const {
    return _swapchain;
}

VkExtent2D Swapchain::getExtent() const {
    const VkExtent3D& extent = _images.at(0).extent;
    return { extent.width, extent.height };
}

const VkFormat Swapchain::getVkFormat() const {
    return _images.at(0).format;
}

const std::vector<Image>& Swapchain::getImages() const {
    return _images;
}

void Swapchain::cleanup() {
    VkDevice device = _logicalDevice.getVkDevice();

    for (auto image : _images) {
        vkDestroyImageView(device, image.view, nullptr);
    }

    vkDestroySwapchainKHR(device, _swapchain, nullptr);
}

void Swapchain::create() {
    const auto& propertyManager = _logicalDevice.getPhysicalDevice().getPropertyManager();

    const SwapChainSupportDetails swapChainSupport = propertyManager.getSwapChainSupportDetails();
    const VkDevice device = _logicalDevice.getVkDevice();
    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, VK_FORMAT_R8G8B8A8_SRGB);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, VK_PRESENT_MODE_MAILBOX_KHR);
    const VkExtent2D windowExtent = _window.getFramebufferSize();
    std::vector<VkImage> images;

    VkExtent2D extent = chooseSwapExtent({ windowExtent.width, windowExtent.height }, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface.getVkSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices& indices = propertyManager.getQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, images.data());

    _images.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        _images[i] = {
            .image  = images[i],
            .memory = VK_NULL_HANDLE,
            .view   = _logicalDevice.createImageView(images[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1),
            .format = surfaceFormat.format,
            .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .extent = { extent.width, extent.height, 1 },
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT
        };
    }
}

void Swapchain::recrete() {
    cleanup();
    create();
}

VkResult Swapchain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const {
    return vkAcquireNextImageKHR(_logicalDevice.getVkDevice(), _swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
}

VkResult Swapchain::present(uint32_t imageIndex, VkSemaphore waitSemaphore) const {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(_logicalDevice.presentQueue, &presentInfo);
}
