#pragma once

#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class LogicalDevice;
class Renderpass;

// std::vector<std::unique_ptr<Texture2D>> createTexturesFromRenderpass(const LogicalDevice& logicalDevice, const Renderpass& renderpass, const VkExtent2D& extent);

class Framebuffer {
	VkFramebuffer _framebuffer;

	const Renderpass& _renderPass;

public:
	Framebuffer(const Renderpass& renderpass, const VkExtent2D& extent, const std::vector<VkImageView>& imageViews);
	~Framebuffer();

	VkFramebuffer getVkFramebuffer() const;
};
