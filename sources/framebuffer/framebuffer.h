#pragma once

#include "memory_objects/texture/texture_factory.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class CommandPool;
class Renderpass;
class Swapchain;

class Framebuffer {
	VkFramebuffer _framebuffer;
	std::vector<std::unique_ptr<Texture>> _textureAttachments;

	const Renderpass& _renderpass;

public:
	// Framebuffer for presenting images to the screen.
	Framebuffer(const Renderpass& renderpass, const Swapchain& swapchain, uint8_t swapchianIndex, const CommandPool& commandPool);
	// Ofscreen framebuffer.
	Framebuffer(const Renderpass& renderpass, VkExtent2D extent, const std::vector<VkImageView>& imageViews);
	~Framebuffer();

	VkFramebuffer getVkFramebuffer() const;
};
