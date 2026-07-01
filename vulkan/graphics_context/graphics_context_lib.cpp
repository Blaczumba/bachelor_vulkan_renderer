#include "vulkan/graphics_context/graphics_context_lib.h"

#include <ranges>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/memory_objects/image.h"

namespace vlkn::internal {

// Framebuffer createFramebufferFromTextures(
//     const Renderpass& renderpass, std::span<const Image> textures) {
//   std::vector<VkImageView> imageViews;
//   imageViews.reserve(textures.size());
//   std::optional<VkExtent2D> extent;
//   for (const Image& texture : textures) {
//     imageViews.push_back(texture.getVkImageView());
//     if (!extent.has_value()) {
//       extent = texture.getVkExtent2D();
//     } else if (VkExtent2D tmpExtent = texture.getVkExtent2D();
//                extent->width != tmpExtent.width || extent->height != tmpExtent.height) {
//       throw EngineException("All images must have the same size to create a Framebuffer.");
//     }
//   }
//
//   if (!extent.has_value()) {
//     throw EngineException("Framebuffer must have an attachment.");
//   }
//
//   return Framebuffer::create(renderpass, *extent, imageViews);
// }

lib::Buffer<VkDescriptorPoolSize> getDescriptorPoolSizesFromBindings(
    std::span<const std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> bindings) {
  std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
  for (const VkDescriptorSetLayoutBinding& binding : std::views::keys(bindings)) {
    descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
  }
  lib::Buffer<VkDescriptorPoolSize> poolSizes(descriptorTypeCounts.size());
  uint32_t index = 0;
  for (const auto [VkDescriptorType, uint32_t] : descriptorTypeCounts) {
    poolSizes[index++] =
        VkDescriptorPoolSize{.type = VkDescriptorType, .descriptorCount = uint32_t};
  }
  return poolSizes;
}

}  // namespace vlkn::internal
