#pragma once

#include <span>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/util/resource_handles.h"
#include "lib/buffer/buffer.h"
#include "vulkan/resource_manager/gpu_buffer_manager.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"

class FramebufferAttachmentManager {
public:
  FramebufferAttachmentManager(GpuBufferManager& gpuBufferManager) noexcept;

  ~FramebufferAttachmentManager();

  Framebuffer createFramebuffer(
      const Renderpass& renderpass, std::span<const GpuImageHandle> attachments, VkExtent2D extent,
      VkImageView swapchainView = VK_NULL_HANDLE);

  std::span<const GpuImageHandle> getAttachments(VkFramebuffer framebuffer) const;

private:
  GpuBufferManager& _gpuBufferManager;

  std::unordered_map<VkFramebuffer, lib::Buffer<GpuImageHandle>> _framebuffersAttachments;
  std::unordered_map<VkFramebuffer, VkImageView> _framebuffersSwapchainViews;
};
