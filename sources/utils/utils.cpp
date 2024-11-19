#include "utils.h"

#include "memory_objects/texture/texture_factory.h"

#include <stdexcept>

std::vector<std::unique_ptr<Texture>> createTexturesFromRenderpass(const CommandPool& commandPool, const Renderpass& renderpass, const VkExtent2D& extent) {
    const auto& attachments = renderpass.getAttachmentsLayout().getAttachments();
    std::vector<std::unique_ptr<Texture>> framebufferTextures;
    framebufferTextures.reserve(attachments.size());

    const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();

    for (size_t i = 0; i < attachments.size(); i++) {
        const VkAttachmentDescription& description = attachments[i].getDescription();
        switch (description.finalLayout) {

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            framebufferTextures.emplace_back(nullptr);
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            framebufferTextures.emplace_back(TextureFactory::createColorAttachment(commandPool, description.format, description.samples, extent));
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            framebufferTextures.emplace_back(TextureFactory::createDepthAttachment(commandPool, description.format, description.samples, extent));
            break;
        default:
            std::runtime_error("failed to recognize final layout in the framebuffer!");
        }
    }

    return framebufferTextures;
}


std::vector<VkImageView> combineViewsForFramebuffer(const std::vector<std::unique_ptr<Texture>>& attachments, const VkImageView view) {
    std::vector<VkImageView> views;
    std::transform(attachments.cbegin(), attachments.cend(), std::back_inserter(views),
        [view](const std::unique_ptr<Texture>& texture) { return texture != nullptr ? texture->getVkImageView() : view; });
    return views;
}
