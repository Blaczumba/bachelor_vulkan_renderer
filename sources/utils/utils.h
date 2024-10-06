#pragma once

#include "memory_objects/texture/texture.h"
#include "render_pass/render_pass.h"
#include "render_pass/framebuffer/framebuffer.h"

#include <vector>
#include <memory>

std::vector<std::unique_ptr<Texture>> createTexturesFromRenderpass(const Renderpass& renderpass, const VkExtent2D& extent);
