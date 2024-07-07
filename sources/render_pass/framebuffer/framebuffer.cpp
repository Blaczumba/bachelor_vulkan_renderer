#include "framebuffer.h"

#include "memory_objects/image/image_color.h"
#include "memory_objects/image/image_depth.h"

#include <stdexcept>

//TODO
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

Framebuffer::Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Renderpass> renderpass)
    : _logicalDevice(logicaldevice), _swapchain(swapchain), _renderPass(renderpass) {

    const auto& swapchainImageViews = _swapchain->getImageViews();
    const auto& swapchainExtent = _swapchain->getExtent();
    const auto& attachments = _renderPass->getAttachments();

    _framebuffers.resize(swapchainImageViews.size());
    _images.reserve(swapchainImageViews.size());

    size_t swapchainPlace{};
    std::vector<VkImageView> attachmentViews;
    for (size_t i = 0; i < attachments.size(); i++) {
        const VkAttachmentDescription& description = attachments[i]->getDescription();
        switch (description.finalLayout) {

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            // TODO adding swapchain images to _images
            attachmentViews.push_back(VK_NULL_HANDLE);
            swapchainPlace = i;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            _images.emplace_back(new ImageColor(_logicalDevice, description.format, description.samples, swapchainExtent));
            attachmentViews.push_back(_images.back()->getVkImageView());
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            _images.emplace_back(new ImageDepth(_logicalDevice, description.format, description.samples, swapchainExtent));
            attachmentViews.push_back(_images.back()->getVkImageView());
            break;
        default:
            std::runtime_error("failed to recognize final layout in framebuffer!");
        }
    }

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::vector<VkImageView> views = attachmentViews;
        views[swapchainPlace] = swapchainImageViews[i];

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass->getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(views.size());
        framebufferInfo.pAttachments = views.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
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

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
