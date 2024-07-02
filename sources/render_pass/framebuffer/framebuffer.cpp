#include "framebuffer.h"

#include <stdexcept>

Framebuffer::Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::vector<std::vector<VkImageView>>&& images, std::shared_ptr<Renderpass> renderpass, VkExtent2D extent, uint32_t count)
    : _logicalDevice(logicaldevice), _renderPass(renderpass) {

    _framebuffers.resize(count);

    for (size_t i = 0; i < count; i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass->getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(images[i].size());
        framebufferInfo.pAttachments = images[i].data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_logicalDevice->getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

std::vector<VkFramebuffer> Framebuffer::getVkFramebuffers() const {
    return _framebuffers;
}

Framebuffer::~Framebuffer() {
    VkDevice device = _logicalDevice->getVkDevice();

    for (auto& image : _images) {
        vkDestroyImageView(device, image._resourcesView, nullptr);
        vkDestroyImage(device, image._resourcesImage, nullptr);
        vkFreeMemory(device, image._resourcesMemory, nullptr);
    }

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

SwapchainImage Framebuffer::createColorResources(const VkAttachmentDescription& description) {
    VkExtent2D swapchainExtent = _swapchain->getExtent();
    VkFormat colorFormat = description.format;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    _logicalDevice->createImage(swapchainExtent.width, swapchainExtent.height, 1, description.samples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = _logicalDevice->createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    return { colorImage, colorImageMemory, colorImageView, swapchainExtent };
}

SwapchainImage Framebuffer::createDepthResources(const VkAttachmentDescription& description) {
    VkExtent2D swapchainExtent = _swapchain->getExtent();
    VkFormat depthFormat = description.format;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    _logicalDevice->createImage(swapchainExtent.width, swapchainExtent.height, 1, description.samples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = _logicalDevice->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    return { depthImage, depthImageMemory, depthImageView, swapchainExtent };
}
