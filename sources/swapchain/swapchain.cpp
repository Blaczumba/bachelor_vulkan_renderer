#include "swapchain.h"
#include "features/swapchain_utils.h"

#include <algorithm>
#include <stdexcept>

Swapchain::Swapchain(std::shared_ptr<Surface> surface, std::shared_ptr<Window> window, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<PhysicalDevice> physicalDevice)
	: _surface(surface), _window(window), _logicalDevice(logicalDevice), _physicalDevice(physicalDevice) {
    create();
}

Swapchain::~Swapchain() {
    cleanup();
}


const std::vector<VkImageView>& Swapchain::getImageViews() const {
    return _imageViews;
}

const VkExtent2D& Swapchain::getExtent() const {
    return _extent;
}

void Swapchain::cleanup() {
    VkDevice device = _logicalDevice->getVkDevice();

    for (auto imageView : _imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, _swapchain, nullptr);
}

void Swapchain::create() {
    SwapChainSupportDetails swapChainSupport = _physicalDevice->getSwapChainSupportDetails();
    VkDevice device = _logicalDevice->getVkDevice();
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, VK_FORMAT_B8G8R8A8_SRGB);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, VK_PRESENT_MODE_MAILBOX_KHR);
    Extent windowExtent = _window->getFramebufferSize();
    VkExtent2D extent = chooseSwapExtent({ windowExtent.width, windowExtent.height }, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface->getVkSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = _physicalDevice->getQueueFamilyIndices();
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
    _images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, _images.data());

    _imageViews.resize(imageCount);
    _imageFormat = surfaceFormat.format;
    _extent = extent;

    for (size_t i = 0; i < imageCount; i++) {
        _imageViews[i] = _logicalDevice->createImageView(_images[i], _imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Swapchain::recrete() {
    cleanup();
    create();
}