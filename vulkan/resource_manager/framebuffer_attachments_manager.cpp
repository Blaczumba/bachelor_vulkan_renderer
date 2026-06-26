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
    for (GpuImageHandle imageHandle : attachments.attachments) {
      _gpuBufferManager.decreaseRefCount(imageHandle);
    }
  }
}

Framebuffer FramebufferAttachmentManager::createFramebuffer(
    const Renderpass& renderpass, std::span<const GpuImageHandle> attachments, VkExtent2D extent,
    VkImageView swapchainView) {
  std::vector<VkImageView> views;
  if (swapchainView != VK_NULL_HANDLE) {
    views.push_back(swapchainView);
  }

  for (GpuImageHandle imageHandle : attachments) {
    _gpuBufferManager.increaseRefCount(imageHandle);
    views.push_back(_gpuBufferManager.getImage(imageHandle).getVkImageView());
  }

  Framebuffer framebuffer = Framebuffer::create(renderpass, extent, views);
  _framebuffers.emplace(
      framebuffer.getVkFramebuffer(),
      Attachments{.attachments = attachments, .swapchainImageView = swapchainView});
  return framebuffer;
}

std::span<const GpuImageHandle> FramebufferAttachmentManager::getAttachments(
    VkFramebuffer framebuffer) {
  auto it = _framebuffers.find(framebuffer);
  if (it == _framebuffers.end()) [[unlikely]] {
    throw EngineException("Framebuffer not found in FramebufferAttachmentManager.");
  }
  return it->second.attachments;
}
