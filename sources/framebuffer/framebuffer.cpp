#include "framebuffer.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>

Framebuffer::Framebuffer(const Renderpass& renderpass, const Swapchain& swapchain, uint8_t swapchainIndex, const CommandPool& commandPool)
    : _renderpass(renderpass), _swapchainIndex(swapchainIndex), _extent(swapchain.getExtent()) {
    if (swapchain.getImagesCount() <= _swapchainIndex) {
        throw std::runtime_error("swapchainIndex does not fit in swapchain images count!");
    }
    _viewport = VkViewport{
        .width = static_cast<float>(_extent.width),
        .height = static_cast<float>(_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    _scissor = VkRect2D{
        .extent = _extent
    };

    const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();
    const std::vector<VkAttachmentDescription>& descriptions = _renderpass.getAttachmentsLayout().getVkAttachmentDescriptions();

    std::vector<VkImageView> imageViews;

    SingleTimeCommandBuffer handle(commandPool);
    const VkCommandBuffer commandBuffer = handle.getCommandBuffer();
    for (const auto& description : descriptions) {
        switch (description.finalLayout) {
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imageViews.push_back(swapchain.getVkImageViews()[*_swapchainIndex]);
            continue;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createColorAttachment(logicalDevice, commandBuffer, description.format, description.samples, _extent));
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createDepthAttachment(logicalDevice, commandBuffer, description.format, description.samples, _extent));
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
        .width = _extent.width,
        .height = _extent.height,
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
    _extent = *extent;
    _viewport = VkViewport{
        .width = static_cast<float>(_extent.width),
        .height = static_cast<float>(_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    _scissor = VkRect2D{
        .extent = _extent
    };

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
    vkDestroyFramebuffer(_renderpass.getLogicalDevice().getVkDevice(), _framebuffer, nullptr);
}

VkExtent2D Framebuffer::getVkExtent() const {
    return _extent;
}

const VkViewport& Framebuffer::getViewport() const {
    return _viewport;
}

const VkRect2D& Framebuffer::getScissor() const {
    return _scissor;
}

const Renderpass& Framebuffer::getRenderpass() const {
    return _renderpass;
}

VkFramebuffer Framebuffer::getVkFramebuffer() const {
    return _framebuffer;
}
