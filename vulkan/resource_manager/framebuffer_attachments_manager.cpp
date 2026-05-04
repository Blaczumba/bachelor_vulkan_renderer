#include "vulkan/resource_manager/framebuffer_attachments_manager.h"

#include <span>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "common/util/resource_handles.h"
#include "vulkan/resource_manager/gpu_buffer_manager.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"

FramebufferAttachmentManager::FramebufferAttachmentManager(
    GpuBufferManager& gpuBufferManager) noexcept
  : _gpuBufferManager(gpuBufferManager) {}

FramebufferAttachmentManager::~FramebufferAttachmentManager() {
  for (const auto& [framebuffer, attachments] : _framebuffers) {
    for (GpuTextureHandle textureHandle : attachments.attachments) {
      _gpuBufferManager.decreaseRefCount(textureHandle);
    }
  }
}

Framebuffer FramebufferAttachmentManager::createFramebuffer(
    const Renderpass& renderpass, std::span<const GpuTextureHandle> attachments, VkExtent2D extent,
    VkImageView swapchainView) {
  std::vector<VkImageView> views;
  if (swapchainView != VK_NULL_HANDLE) {
    views.push_back(swapchainView);
  }

  for (GpuTextureHandle textureHandle : attachments) {
    _gpuBufferManager.increaseRefCount(textureHandle);
    views.push_back(_gpuBufferManager.getTexture(textureHandle).getVkImageView());
  }

  Framebuffer framebuffer = Framebuffer::create(renderpass, extent, 1, views);
  _framebuffers.emplace(
      framebuffer.getVkFramebuffer(),
      Attachments{.attachments = attachments, .swapchainImageView = swapchainView});
  return framebuffer;
}

std::span<const GpuTextureHandle> FramebufferAttachmentManager::getAttachments(
    VkFramebuffer framebuffer) {
  auto it = _framebuffers.find(framebuffer);
  if (it == _framebuffers.end()) [[unlikely]] {
    throw EngineException("Framebuffer not found in FramebufferAttachmentManager.");
  }
  return it->second.attachments;
}
