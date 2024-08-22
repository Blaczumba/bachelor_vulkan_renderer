#pragma once

#include "subpass/subpass.h"

#include <vector>
#include <variant>
#include <memory>

class LogicalDevice;

class Renderpass {
	VkRenderPass _renderpass = VK_NULL_HANDLE;
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

	const VkRenderPass getVkRenderPass() const;
	const std::vector<VkClearValue>& getClearValues() const;
	const AttachmentLayout& getAttachmentsLayout() const;
	uint32_t getColorAttachmentsCount() const;

	void addSubpass(const Subpass& subpass);
	void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask);
};
