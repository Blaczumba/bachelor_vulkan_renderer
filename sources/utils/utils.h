#pragma once

#include "command_buffer/command_buffer.h"
#include "memory_objects/texture/texture.h"
#include "render_pass/render_pass.h"
#include "render_pass/framebuffer/framebuffer.h"

#include <vector>
#include <memory>

std::vector<std::unique_ptr<Texture>> createTexturesFromRenderpass(const CommandPool& commandPool, const Renderpass& renderpass, const VkExtent2D& extent);
std::vector<VkImageView> combineViewsForFramebuffer(const std::vector<std::unique_ptr<Texture>>& attachments, const VkImageView view);