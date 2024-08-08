#include "render_pass.h"

#include "logical_device/logical_device.h"

#include <stdexcept>
#include <iostream>

Renderpass::Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const AttachmentLayout& layout) 
    : _logicalDevice(logicalDevice), _attachmentsLayout(layout) {
    _clearValues = _attachmentsLayout.getVkClearValues();
    _colorAttachmentsCount = _attachmentsLayout.getColorAttachmentsCount();
}

void Renderpass::create() {
    if (_renderpass)
        throw std::runtime_error("Renderpass has already been created!");

    std::vector<VkAttachmentDescription> attachmentDescriptions = _attachmentsLayout.getVkAttachmentDescriptions();

    std::vector<VkSubpassDescription> subpassDescriptions;
    std::transform(_subpasses.cbegin(), _subpasses.cend(), std::back_inserter(subpassDescriptions), [](const Subpass& subpass) { return subpass.getVkSubpassDescription(); });

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(_subpassDepencies.size());
    renderPassInfo.pDependencies = _subpassDepencies.data();

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

const AttachmentLayout& Renderpass::getAttachmentsLayout() const {
    return _attachmentsLayout;
}

uint32_t Renderpass::getColorAttachmentsCount() const {
    return _colorAttachmentsCount;
}

void Renderpass::addSubpass(const Subpass& subpass) {
    _subpasses.push_back(subpass);
}

void Renderpass::addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask) {
    VkSubpassDependency dependency{};
    dependency.srcSubpass = srcSubpassIndex;
    dependency.dstSubpass = dstSubpassIndex;
    dependency.srcStageMask = srcStageMask;
    dependency.srcAccessMask = srcAccessMask;
    dependency.dstStageMask = dstStageMask;
    dependency.dstAccessMask = dstAccessMask;

    _subpassDepencies.push_back(dependency);
}
