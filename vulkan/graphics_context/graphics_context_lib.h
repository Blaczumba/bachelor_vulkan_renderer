#pragma once

#include <span>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/command_buffer/command_buffer.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/memory_objects/image.h"

namespace vlkn::internal {

// Framebuffer createFramebufferFromTextures(
//     const Renderpass& renderpass, std::span<const Image> textures);

lib::Buffer<VkDescriptorPoolSize> getDescriptorPoolSizesFromBindings(
    std::span<const std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> bindings);

}  // namespace vlkn::internal
