#include "framebuffer.h"

#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>

Framebuffer::Framebuffer(const Renderpass& renderpass, const Swapchain& swapchain, uint8_t swapchainIndex, const CommandPool& commandPool)
    : _renderpass(renderpass), _swapchainIndex(swapchainIndex) {
    if (swapchain.getImagesCount() <= _swapchainIndex)
        throw std::runtime_error("swapchainIndex does not fit in swapchain images count!");

    const std::vector<Attachment>& attachments = _renderpass.getAttachmentsLayout().getAttachments();
    std::vector<VkAttachmentDescription> descriptions;
    std::transform(attachments.cbegin(), attachments.cend(), std::back_inserter(descriptions), [](const Attachment& attachment) { return attachment.getDescription(); });

    std::vector<VkImageView> imageViews;
    const VkExtent2D swapchainExtent = swapchain.getExtent();
    for (const auto& description : descriptions) {
        switch (description.finalLayout) {
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imageViews.push_back(swapchain.getVkImageViews()[*_swapchainIndex]);
            continue;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createColorAttachment(commandPool, description.format, description.samples, swapchainExtent));
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createDepthAttachment(commandPool, description.format, description.samples, swapchainExtent));
            break;
        default:
            throw std::runtime_error("failed to recognize final layout in the framebuffer!");
        }
        imageViews.push_back(_textureAttachments.back()->getVkImageView());
    }

    const VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderpass.getVkRenderPass(),
        .attachmentCount = static_cast<uint32_t>(imageViews.size()),
        .pAttachments = imageViews.data(),
        .width = swapchainExtent.width,
        .height = swapchainExtent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(_renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::Framebuffer(const Renderpass& renderpass, std::vector<std::shared_ptr<Texture>>&& textures)
    : _renderpass(renderpass), _textureAttachments(std::move(textures)) {
    std::vector<VkImageView> imageViews;
    imageViews.reserve(_textureAttachments.size());
    std::optional<VkExtent2D> extent;
    for (const auto& texture : _textureAttachments) {
        imageViews.emplace_back(texture->getVkImageView());
        if (!extent.has_value()) {
            extent = texture->getVkExtent2D();
        } else if (VkExtent2D tmpExtent = texture->getVkExtent2D(); extent->width != tmpExtent.width || extent->height != tmpExtent.height) {
            throw std::runtime_error("framebuffer textures differ in extent!");
        }
    }
    if (!extent.has_value()) {
        throw std::runtime_error("framebuffer has no textures specified!");
    }

    const VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderpass.getVkRenderPass(),
        .attachmentCount = static_cast<uint32_t>(imageViews.size()),
        .pAttachments = imageViews.data(),
        .width = extent->width,
        .height = extent->height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(_renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer() {
    VkDevice device = _renderpass.getLogicalDevice().getVkDevice();

    vkDestroyFramebuffer(device, _framebuffer, nullptr);
}

const Renderpass& Framebuffer::getRenderpass() const {
    return _renderpass;
}

VkFramebuffer Framebuffer::getVkFramebuffer() const {
    return _framebuffer;
}
