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
protected:
	std::vector<VkFramebuffer> _framebuffers;
	const LogicalDevice& _logicalDevice;
	const Renderpass& _renderPass;

	std::vector<std::shared_ptr<Texture2DColor>> _colorImages;
	std::vector<std::shared_ptr<Texture2D>> _depthImages;

public:
	Framebuffer(const LogicalDevice& logicaldevice, const Renderpass& renderpass);
	virtual ~Framebuffer() = default;

	const std::vector<std::shared_ptr<Texture2DColor>>& getColorTextures() const;

	std::vector<VkFramebuffer> getVkFramebuffers() const;
};
