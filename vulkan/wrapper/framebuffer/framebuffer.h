#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/render_pass/render_pass.h"

class Framebuffer {
  Framebuffer(VkFramebuffer framebuffer, const Renderpass& renderpass, const VkViewport& viewport,
              const VkRect2D& scissor) noexcept;

public:
  Framebuffer() noexcept = default;

  static Framebuffer create(
      const Renderpass& renderpass, VkExtent2D extent, std::span<const VkImageView> attachments);

  Framebuffer(Framebuffer&& framebuffer) noexcept;

  Framebuffer& operator=(Framebuffer&& framebuffer) noexcept;

  ~Framebuffer();

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
