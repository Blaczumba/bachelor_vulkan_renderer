#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class LogicalDevice;
class Renderpass;

class Framebuffer {
	VkFramebuffer _framebuffer;

	const Renderpass& _renderPass;

public:
	Framebuffer(const Renderpass& renderpass, VkExtent2D extent, const std::vector<VkImageView>& imageViews);
	~Framebuffer();

	VkFramebuffer getVkFramebuffer() const;
};
