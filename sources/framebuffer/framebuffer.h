#pragma once

#include "memory_objects/texture/texture_factory.h"
#include "render_pass/render_pass.h"

#include <vulkan/vulkan.h>

#include <initializer_list>
#include <memory>
#include <vector>

class CommandPool;
class Swapchain;

class Framebuffer {
	VkFramebuffer _framebuffer;
	std::vector<std::shared_ptr<Texture>> _textureAttachments;
	const std::optional<uint8_t> _swapchainIndex;
	VkExtent2D _extent;

	const Renderpass& _renderpass;

public:
	// Framebuffer for presenting images to the screen.
	Framebuffer(const Renderpass& renderpass, const Swapchain& swapchain, uint8_t swapchainIndex, const CommandPool& commandPool);
	// Ofscreen framebuffer requires all textures to have the same size.
	Framebuffer(const Renderpass& renderpass, std::vector<std::shared_ptr<Texture>>&& textures);
	~Framebuffer();

	VkExtent2D getVkExtent() const;
	const Renderpass& getRenderpass() const;
	VkFramebuffer getVkFramebuffer() const;
};
