#include "render_pass.h"
#include "attachment/attachment.h"

#include <stdexcept>
#include <iostream>

Renderpass::Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<Attachment>& attachments)
    : _logicalDevice(logicalDevice) {
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    std::vector<VkAttachmentReference> depthAttachmentRefs;
    std::vector<VkAttachmentReference> colorAttachmentResolveRefs;
    std::vector<VkAttachmentDescription> attachmentDescriptions;

    for (uint32_t i = 0; i < attachments.size(); i++) {
        Attachment attachment = attachments[i];

        VkAttachmentReference attachmentRefeference{};
        attachmentRefeference.attachment = i;
        VkClearValue clearValue{};

        if (const auto ref = std::get_if<ColorAttachment>(&attachment)) {
            const VkAttachmentDescription& description = ref->description;
            attachmentDescriptions.push_back(description);
            attachmentRefeference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.push_back(attachmentRefeference);
            
            clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            _clearValues.push_back(clearValue);

            _layout.emplace_back(description);
        }
        else if (const auto ref = std::get_if<DepthAttachment>(&attachment)) {
            const VkAttachmentDescription& description = ref->description;
            attachmentDescriptions.push_back(description);
            attachmentRefeference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentRefs.push_back(attachmentRefeference);

            clearValue.depthStencil = { 1.0f, 0 };
            _clearValues.push_back(clearValue);

            _layout.emplace_back(description);
        }
        else if (const auto ref = std::get_if<ColorAttachmentResolve>(&attachment)) {
            const VkAttachmentDescription& description = ref->description;
            attachmentDescriptions.push_back(description);
            attachmentRefeference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentResolveRefs.push_back(attachmentRefeference);

            clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            _clearValues.push_back(clearValue);

            _layout.emplace_back(description);
        }
        else {
            throw std::runtime_error("failed to recognize attachment type in render pass creation");
        }
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
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

const std::vector<VkAttachmentDescription>& Renderpass::getLayout() const {
    return _layout;
}
