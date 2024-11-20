#pragma once

#include "logical_device/logical_device.h"
#include "subpass/subpass.h"
#include "memory_objects/texture/texture_factory.h"
#include "render_pass/attachment/attachment_layout.h"

#include <memory>
#include <vector>

class CommandPool;
class LogicalDevice;

class Renderpass {
	VkRenderPass _renderpass = VK_NULL_HANDLE;
	AttachmentLayout _attachmentsLayout;

	std::vector<Subpass> _subpasses;
	std::vector<VkSubpassDependency> _subpassDepencies;
	std::vector<VkClearValue> _clearValues;

	const LogicalDevice& _logicalDevice;

public:
	Renderpass(const LogicalDevice& logicalDevice, const AttachmentLayout& layout);
	~Renderpass();
	void create();
	void cleanup();

	const VkRenderPass getVkRenderPass() const;
	const std::vector<VkClearValue>& getClearValues() const;
	const AttachmentLayout& getAttachmentsLayout() const;

	void addSubpass(const Subpass& subpass);
	void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask);
	std::vector<std::unique_ptr<Texture>> createTexturesFromRenderpass(const CommandPool& commandPool, const VkExtent2D& extent);
	const LogicalDevice& getLogicalDevice() const;
};
