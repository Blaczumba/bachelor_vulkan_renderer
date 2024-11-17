#include "utils.h"

#include "memory_objects/texture/texture_factory.h"

#include <stdexcept>

std::vector<std::unique_ptr<Texture>> createTexturesFromRenderpass(const Renderpass& renderpass, const VkExtent2D& extent) {
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
            framebufferTextures.emplace_back(TextureFactory::createColorAttachment(logicalDevice, description.format, description.samples, extent));
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            framebufferTextures.emplace_back(TextureFactory::createDepthAttachment(logicalDevice, description.format, description.samples, extent));
            break;
        default:
            std::runtime_error("failed to recognize final layout in the framebuffer!");
        }
    }

    return framebufferTextures;
}
