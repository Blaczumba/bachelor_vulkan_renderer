#include "framebuffer_from_textures.h"

#include <stdexcept>

FramebufferFromTextures::FramebufferFromTextures(const LogicalDevice& logicalDevice, const Renderpass& renderpass, std::vector<std::vector<std::shared_ptr<Texture2D>>>&& images)
    : Framebuffer(logicalDevice, renderpass) {

    VkExtent3D extent3D = images[0][0]->getImage().extent;
    VkExtent2D extent = { extent3D.width, extent3D.height };

    size_t count = images.size();
    _framebuffers.resize(count);

    std::vector<std::vector<VkImageView>> imageViews;

    for (auto& row : images) {
        imageViews.push_back({});
        for (auto& image : row) {
            const Image& img = image->getImage();
            imageViews.back().push_back(img.view);
            if(img.extent.width != extent.width || img.extent.height != extent.width)
                std::runtime_error("textures differ in extent (width, height)!");

            switch (img.layout) {
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                _colorImages.emplace_back(std::dynamic_pointer_cast<Texture2DColor>(std::move(image))); // TODO maybe get rid of those texture types
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                _depthImages.emplace_back(std::move(image));
                break;
            default:
                std::runtime_error("failed to recognize final layout in the framebuffer!");
            }
        }
    }

    for (size_t i = 0; i < count; i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass.getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews[i].size());
        framebufferInfo.pAttachments = imageViews[i].data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_logicalDevice.getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

FramebufferFromTextures::~FramebufferFromTextures() {
    VkDevice device = _logicalDevice.getVkDevice();

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
