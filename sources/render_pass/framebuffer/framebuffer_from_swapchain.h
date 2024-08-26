#pragma once

#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"
#include "render_pass/render_pass.h"

#include "memory_objects/texture/texture_2D_color.h"
#include "memory_objects/texture/texture_2D_depth.h"

#include "framebuffer.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class FramebufferFromSwapchain : public Framebuffer {
	const Swapchain& _swapchain;

public:
	FramebufferFromSwapchain(const LogicalDevice& logicalDevice, const Swapchain& swapchain, const Renderpass& renderpass);
	~FramebufferFromSwapchain();
};
