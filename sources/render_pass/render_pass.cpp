#include "render_pass.h"
#include "attachment/attachment.h"

#include <stdexcept>
#include <iostream>

Renderpass::Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, std::vector<std::unique_ptr<Attachment>>&& attachments)
    : _logicalDevice(logicalDevice), _attachments(std::move(attachments)) {
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    std::vector<VkAttachmentReference> depthAttachmentRefs;
    std::vector<VkAttachmentReference> colorAttachmentResolveRefs;
    std::vector<VkAttachmentDescription> attachmentDescriptions;

    for (uint32_t i = 0; i < _attachments.size(); i++) {
        const Attachment* attachment = _attachments[i].get();

        VkAttachmentReference attachmentReference{};
        attachmentReference.layout = attachment->getLayout();
        attachmentReference.attachment = i;

        attachmentDescriptions.push_back(attachment->getDescription());
        VkClearValue clearValue = attachment->getClearValue();

        if (dynamic_cast<const ColorAttachment*>(attachment) || dynamic_cast<const ColorPresentAttachment*>(attachment)) {
            colorAttachmentRefs.push_back(attachmentReference);
        }
        else if (dynamic_cast<const DepthAttachment*>(attachment)) {
            depthAttachmentRefs.push_back(attachmentReference);
        }
        else if (dynamic_cast<const ColorResolveAttachment*>(attachment) || dynamic_cast<const ColorResolvePresentAttachment*>(attachment)) {
            colorAttachmentResolveRefs.push_back(attachmentReference);
        }
        else {
            throw std::runtime_error("Unknown attachment type");
        }

        _clearValues.push_back(clearValue);
    }

    _colorAttachmentsCount = static_cast<uint32_t>(colorAttachmentRefs.size());

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = _colorAttachmentsCount;
    if(!colorAttachmentRefs.empty())
        subpass.pColorAttachments = colorAttachmentRefs.data();
    if(!depthAttachmentRefs.empty())
        subpass.pDepthStencilAttachment = depthAttachmentRefs.data();
    if(!colorAttachmentResolveRefs.empty())
        subpass.pResolveAttachments = colorAttachmentResolveRefs.data();

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
  
    if (vkCreateRenderPass(_logicalDevice->getVkDevice(), &renderPassInfo, nullptr, &_renderpass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

Renderpass::~Renderpass() {
    vkDestroyRenderPass(_logicalDevice->getVkDevice(), _renderpass, nullptr);
}

VkRenderPass Renderpass::getVkRenderPass() {
    return _renderpass;
}

const std::vector<VkClearValue>& Renderpass::getClearValues() const {
    return _clearValues;
}

const std::vector<std::unique_ptr<Attachment>>& Renderpass::getAttachments() const {
    return std::move(_attachments);
}

uint32_t Renderpass::getColorAttachmentsCount() const {
    return _colorAttachmentsCount;
}
