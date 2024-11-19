#include "render_pass.h"

#include <iostream>
#include <stdexcept>

Renderpass::Renderpass(const LogicalDevice& logicalDevice, const AttachmentLayout& layout) 
    : _logicalDevice(logicalDevice), _attachmentsLayout(layout) {
    _clearValues = _attachmentsLayout.getVkClearValues();
}

void Renderpass::create() {
    cleanup();

    const std::vector<VkAttachmentDescription> attachmentDescriptions = _attachmentsLayout.getVkAttachmentDescriptions();
    std::vector<VkSubpassDescription> subpassDescriptions;
    subpassDescriptions.reserve(_subpasses.size());
    std::transform(_subpasses.cbegin(), _subpasses.cend(), std::back_inserter(subpassDescriptions), [](const Subpass& subpass) { return subpass.getVkSubpassDescription(); });

    const VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
        .pAttachments = attachmentDescriptions.data(),
        .subpassCount = static_cast<uint32_t>(subpassDescriptions.size()),
        .pSubpasses = subpassDescriptions.data(),
        .dependencyCount = static_cast<uint32_t>(_subpassDepencies.size()),
        .pDependencies = _subpassDepencies.data()
    };

    if (vkCreateRenderPass(_logicalDevice.getVkDevice(), &renderPassInfo, nullptr, &_renderpass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Renderpass::cleanup() {
    if (_renderpass != VK_NULL_HANDLE)
        vkDestroyRenderPass(_logicalDevice.getVkDevice(), _renderpass, nullptr);

    _renderpass = VK_NULL_HANDLE;
}

Renderpass::~Renderpass() {
    cleanup();
}

const VkRenderPass Renderpass::getVkRenderPass() const {
    return _renderpass;
}

const std::vector<VkClearValue>& Renderpass::getClearValues() const {
    return _clearValues;
}

const AttachmentLayout& Renderpass::getAttachmentsLayout() const {
    return _attachmentsLayout;
}

void Renderpass::addSubpass(const Subpass& subpass) {
    _subpasses.push_back(subpass);
}

void Renderpass::addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask) {
    const VkSubpassDependency dependency = {
        .srcSubpass = srcSubpassIndex,
        .dstSubpass = dstSubpassIndex,
        .srcStageMask = srcStageMask,
        .dstStageMask = dstStageMask,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask
    };

    _subpassDepencies.push_back(dependency);
}

std::vector<std::unique_ptr<Texture>> Renderpass::createTexturesFromRenderpass(const CommandPool& commandPool, const VkExtent2D& extent) {
    const auto& attachments = _attachmentsLayout.getAttachments();
    std::vector<std::unique_ptr<Texture>> framebufferTextures;
    framebufferTextures.reserve(attachments.size());

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

const LogicalDevice& Renderpass::getLogicalDevice() const {
    return _logicalDevice;
}
