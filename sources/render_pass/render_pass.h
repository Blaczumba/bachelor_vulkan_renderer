#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"
#include "subpass/subpass.h"

#include <vector>
#include <variant>
#include <memory>

class Renderpass {
	VkRenderPass _renderpass = nullptr;
	AttachmentLayout _attachmentsLayout;
	uint32_t _colorAttachmentsCount;

	std::vector<Subpass> _subpasses;
	std::vector<VkSubpassDependency> _subpassDepencies;
	std::vector<VkClearValue> _clearValues;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const AttachmentLayout& layout);
	~Renderpass();
	void create();

	VkRenderPass getVkRenderPass();
	const std::vector<VkClearValue>& getClearValues() const;
	const AttachmentLayout& getAttachmentsLayout() const;
	uint32_t getColorAttachmentsCount() const;

	void addSubpass(const Subpass& subpass);
	void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask);
};
