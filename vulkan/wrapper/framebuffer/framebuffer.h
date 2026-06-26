#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/render_pass/render_pass.h"

class Framebuffer {
  Framebuffer(const Renderpass& renderpass, VkFramebuffer framebuffer, const VkViewport& viewport,
              const VkRect2D& scissor) noexcept;

public:
  Framebuffer() noexcept = default;

  Framebuffer(Framebuffer&& framebuffer) noexcept;

  Framebuffer& operator=(Framebuffer&& framebuffer) noexcept;

  ~Framebuffer();

  static Framebuffer create(
      const Renderpass& renderpass, const VkFramebufferCreateInfo& createInfo);

  VkExtent2D getVkExtent() const noexcept;

  const VkViewport& getViewport() const noexcept;

  const VkRect2D& getScissor() const noexcept;

  const Renderpass& getRenderpass() const;

  VkFramebuffer getVkFramebuffer() const noexcept;

private:
  void destroy();

  VkFramebuffer _framebuffer = VK_NULL_HANDLE;

  const Renderpass* _renderpass = nullptr;

  VkViewport _viewport;
  VkRect2D _scissor;
};

struct FramebufferMetadata {
  VkExtent2D extent;
  uint32_t layers;
  VkFramebufferCreateFlags flags;
  std::vector<VkImageView> attachments;
  // Other std::optional fields representing pNext metadata.
};

class FramebufferBuilder {
public:
  FramebufferBuilder& addAttachment(VkImageView attachment);

  FramebufferBuilder& withExtent(VkExtent2D extent) noexcept;

  FramebufferBuilder& withLayers(uint32_t layers) noexcept;

  FramebufferBuilder& withFlags(VkFramebufferCreateFlags flags) noexcept;

  FramebufferMetadata getMetadata() const noexcept;

  Framebuffer build(const Renderpass& renderpass) const;

private:
  FramebufferMetadata _metadata = {
    .layers = 1,
  };
};
