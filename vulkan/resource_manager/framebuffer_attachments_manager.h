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

  std::span<const GpuImageHandle> getAttachments(VkFramebuffer framebuffer);

private:
  GpuBufferManager& _gpuBufferManager;

  struct Attachments {
    lib::Buffer<GpuImageHandle> attachments;
    VkImageView swapchainImageView;
  };
  std::unordered_map<VkFramebuffer, Attachments> _framebuffers;
};
