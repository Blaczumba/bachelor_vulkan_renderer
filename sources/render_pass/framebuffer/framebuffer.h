#pragma once

#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"
#include "render_pass/render_pass.h"

#include "memory_objects/texture/texture_2D_color.h"
#include "memory_objects/texture/texture_2D_depth.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Framebuffer {
	std::vector<VkFramebuffer> _framebuffers;
	const LogicalDevice& _logicalDevice;
	const Swapchain* _swapchain;
	const Renderpass& _renderPass;

	std::vector<Texture2DColor> _colorImages;
	std::vector<Texture2DDepth> _depthImages;
public:
	Framebuffer(const LogicalDevice& logicaldevice, std::vector<std::vector<VkImageView>>&& images, const Renderpass& renderpass, VkExtent2D extent, uint32_t count);
	Framebuffer(const LogicalDevice& logicaldevice, const Swapchain* swapchain, const Renderpass& renderpass);
	~Framebuffer();

	const std::vector<Texture2DColor>& getColorTextures() const;

	std::vector<VkFramebuffer> getVkFramebuffers() const;
};
