#pragma once

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"

namespace vlkn::internal {

Framebuffer createFramebufferFromTextures(
    const Renderpass& renderpass, std::span<const Texture> textures) {
  std::vector<VkImageView> imageViews;
  imageViews.reserve(textures.size());
  std::optional<VkExtent2D> extent;
  for (const Texture& texture : textures) {
    imageViews.push_back(texture.getVkImageView());
    if (!extent.has_value()) {
      extent = texture.getVkExtent2D();
    } else if (VkExtent2D tmpExtent = texture.getVkExtent2D();
               extent->width != tmpExtent.width || extent->height != tmpExtent.height) {
      throw EngineException("All images must have the same size to create a Framebuffer.");
    }
  }

  if (!extent.has_value()) {
    throw EngineException("Framebuffer must have an attachment.");
  }

  return Framebuffer::create(renderpass, *extent, imageViews);
}

}  // namespace vlkn::internal
