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
  for (const auto& [framebuffer, attachments] : _framebuffersAttachments) {
    for (GpuImageHandle imageHandle : attachments) {
      _gpuBufferManager.decreaseRefCount(imageHandle);
    }
  }
}

Framebuffer FramebufferAttachmentManager::createFramebuffer(
    const Renderpass& renderpass, std::span<const GpuImageHandle> attachments, VkExtent2D extent,
    VkImageView swapchainView) {
  FramebufferBuilder framebufferBuilder;
  if (swapchainView != VK_NULL_HANDLE) {
    framebufferBuilder.addAttachment(swapchainView);
  }

  for (GpuImageHandle imageHandle : attachments) {
    _gpuBufferManager.increaseRefCount(imageHandle);
    framebufferBuilder.addAttachment(_gpuBufferManager.getImage(imageHandle).getVkImageView());
  }

  Framebuffer framebuffer = framebufferBuilder.build(renderpass, extent, 1);
  _framebuffersAttachments.emplace(framebuffer.getVkFramebuffer(), attachments);
  _framebuffersSwapchainViews.emplace(framebuffer.getVkFramebuffer(), swapchainView);
  return framebuffer;
}

std::span<const GpuImageHandle> FramebufferAttachmentManager::getAttachments(
    VkFramebuffer framebuffer) const {
  auto it = _framebuffersAttachments.find(framebuffer);
  if (it == _framebuffersAttachments.end()) [[unlikely]] {
    throw EngineException("Framebuffer not found in FramebufferAttachmentManager.");
  }
  return it->second;
}
