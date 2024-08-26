#pragma once

#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"
#include "render_pass/render_pass.h"

#include "memory_objects/texture/texture_2D_color.h"
#include "memory_objects/texture/texture_2D_depth.h"

#include "framebuffer.h"

#include <vector>

class FramebufferFromTextures : public Framebuffer {


public:
	FramebufferFromTextures(const LogicalDevice& logicalDevice, const Renderpass& renderpass, std::vector<std::vector<std::shared_ptr<Texture2D>>>&& images);
	~FramebufferFromTextures();
};
