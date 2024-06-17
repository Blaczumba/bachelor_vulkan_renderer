#include "framebuffer.h"

#include <stdexcept>

Framebuffer::Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Renderpass> renderpass)
	: _logicalDevice(logicaldevice), _swapchain(swapchain), _renderPass(renderpass) {

    const auto& swapchainImageViews = _swapchain->getImageViews();
    const auto& swapchainExtent = _swapchain->getExtent();

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass->getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        // resize framebuffers!!!
        if (vkCreateFramebuffer(_logicalDevice->getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Framebuffer::createDepthResources(const VkAttachmentDescription& description, VkImage image, VkDeviceMemory imageMemory, VkImageView imageView) {
    VkExtent2D swapchainExtent = _swapchain->getExtent();
    VkFormat depthFormat = description.format;
    _logicalDevice->createImage(swapchainExtent.width, swapchainExtent.height, 1, description.samples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
    imageView = _logicalDevice->createImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    if (depthFormat == VK_FORMAT_D32_SFLOAT)
        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    else
        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);
}