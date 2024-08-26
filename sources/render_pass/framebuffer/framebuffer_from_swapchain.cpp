#include "framebuffer_from_swapchain.h"

#include <stdexcept>

FramebufferFromSwapchain::FramebufferFromSwapchain(const LogicalDevice& logicalDevice, const Swapchain& swapchain, const Renderpass& renderpass)
    : Framebuffer(logicalDevice, renderpass), _swapchain(swapchain) {
    const auto& images = _swapchain.getImages();
    const auto& swapchainExtent = _swapchain.getExtent();
    const auto& attachments = _renderPass.getAttachmentsLayout().getAttachments();

    std::vector<VkImageView> swapchainImageViews;
    std::transform(images.cbegin(), images.cend(), std::back_inserter(swapchainImageViews), [](const Image& image) { return image.view; });

    _framebuffers.resize(swapchainImageViews.size());

    size_t swapchainPlace{};
    std::vector<VkImageView> attachmentViews;
    _colorImages.reserve(attachments.size());
    _depthImages.reserve(attachments.size());
    for (size_t i = 0; i < attachments.size(); i++) {
        const VkAttachmentDescription& description = attachments[i].getDescription();
        switch (description.finalLayout) {

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            // TODO adding swapchain images to _images
            attachmentViews.push_back(VK_NULL_HANDLE);
            swapchainPlace = i;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            _colorImages.emplace_back(std::make_shared<Texture2DColor>(_logicalDevice, description.format, description.samples, swapchainExtent));
            attachmentViews.push_back(_colorImages.back()->getImage().view);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            _depthImages.emplace_back(std::make_shared<Texture2DDepth>(_logicalDevice, description.format, description.samples, swapchainExtent));
            attachmentViews.push_back(_depthImages.back()->getImage().view);
            break;
        default:
            std::runtime_error("failed to recognize final layout in the framebuffer!");
        }
    }

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::vector<VkImageView> views = attachmentViews;
        views[swapchainPlace] = swapchainImageViews[i];

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass.getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(views.size());
        framebufferInfo.pAttachments = views.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_logicalDevice.getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

FramebufferFromSwapchain::~FramebufferFromSwapchain() {
    VkDevice device = _logicalDevice.getVkDevice();

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
