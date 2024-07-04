#include "swapchain.h"
#include "features/swapchain_utils.h"

#include <algorithm>
#include <stdexcept>

Swapchain::Swapchain(std::shared_ptr<Surface> surface, std::shared_ptr<Window> window, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<PhysicalDevice> physicalDevice)
	: _surface(surface), _window(window), _logicalDevice(logicalDevice), _physicalDevice(physicalDevice), _currentFrame(0) {
    create();
}

Swapchain::~Swapchain() {
    cleanup();
}

const VkSwapchainKHR Swapchain::getVkSwapchain() const {
    return _swapchain;
}

VkFormat Swapchain::getSwapchainImageFormat() const {
    return _imageFormat;
}

const std::vector<VkImageView>& Swapchain::getImageViews() const {
    return _imageViews;
}

const VkExtent2D& Swapchain::getExtent() const {
    return _extent;
}

VkImage Swapchain::getCurrentImage() const {
    return _images[_currentFrame];
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
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, VK_PRESENT_MODE_FIFO_KHR);
    Extent windowExtent = _window->getFramebufferSize();
    VkExtent2D extent = chooseSwapExtent({ windowExtent.width, windowExtent.height }, swapChainSupport.capabilities);

    _imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && _imageCount > swapChainSupport.capabilities.maxImageCount) {
        _imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface->getVkSurface();

    createInfo.minImageCount = _imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

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

    vkGetSwapchainImagesKHR(device, _swapchain, &_imageCount, nullptr);
    _images.resize(_imageCount);
    vkGetSwapchainImagesKHR(device, _swapchain, &_imageCount, _images.data());

    _imageViews.resize(_imageCount);
    _imageFormat = surfaceFormat.format;
    _extent = extent;

    for (size_t i = 0; i < _imageCount; i++) {
        _imageViews[i] = _logicalDevice->createImageView(_images[i], _imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Swapchain::recrete() {
    cleanup();
    create();
}

uint32_t Swapchain::update() {
    if (++_currentFrame == _imageCount)
        _currentFrame = 0;
    return _currentFrame;
}

uint32_t Swapchain::acquireNextImage() const {
    return 0;
}

VkResult Swapchain::present(VkSemaphore waitSemaphore) const {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;
    return VK_SUCCESS;
  /*  presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }*/
}
