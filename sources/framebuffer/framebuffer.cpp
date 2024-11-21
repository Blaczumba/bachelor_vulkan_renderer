#include "framebuffer.h"

#include "logical_device/logical_device.h"
#include "render_pass/render_pass.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

Framebuffer::Framebuffer(const Renderpass& renderpass, VkExtent2D extent, const std::vector<VkImageView>& imageViews)
    : _renderPass(renderpass) {

    VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderPass.getVkRenderPass(),
        .attachmentCount = static_cast<uint32_t>(imageViews.size()),
        .pAttachments = imageViews.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(_renderPass.getLogicalDevice().getVkDevice(), &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer() {
    VkDevice device = _renderPass.getLogicalDevice().getVkDevice();

    vkDestroyFramebuffer(device, _framebuffer, nullptr);
}

VkFramebuffer Framebuffer::getVkFramebuffer() const {
    return _framebuffer;
}
