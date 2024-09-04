#pragma once

#include "memory_objects/texture/texture_2D.h"
#include "render_pass/render_pass.h"
#include "render_pass/framebuffer/framebuffer.h"

#include <vector>
#include <memory>

std::vector<std::unique_ptr<Texture2D>> createTexturesFromRenderpass(const LogicalDevice& logicalDevice, const Renderpass& renderpass, const VkExtent2D& extent);
