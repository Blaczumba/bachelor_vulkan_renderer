#include "vulkan/resource_manager/framebuffer_attachments_manager.h"

#include <span>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <ranges>
#include <numeric>

#include "common/util/engine_exception.h"
#include "common/util/resource_handles.h"
#include "vulkan/resource_manager/gpu_buffer_manager.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"

FramebufferAttachmentManager::FramebufferAttachmentManager(
    GpuBufferManager& gpuBufferManager, std::vector<FramebufferHandle>&& freeFramebuffers) noexcept
  : _gpuBufferManager(gpuBufferManager), _freeFramebufferHandles(std::move(freeFramebuffers)) {}

std::unique_ptr<FramebufferAttachmentManager> FramebufferAttachmentManager::create(
    GpuBufferManager& gpuBufferManager) {
  std::vector<FramebufferHandle> freeFramebufferHandles(MAX_FRAMEBUFFERS);
  std::iota(freeFramebufferHandles.rbegin(), freeFramebufferHandles.rend(), FramebufferHandle(0));
  return std::unique_ptr<FramebufferAttachmentManager>(new FramebufferAttachmentManager(gpuBufferManager, std::move(freeFramebufferHandles)));
}

FramebufferAttachmentManager::~FramebufferAttachmentManager() {
  for (const FramebufferData& framebufferData : std::views::values(_framebuffers.getValues())) {
    for (GpuImageHandle imageHandle : framebufferData.imageHandles) {
      _gpuBufferManager.decreaseRefCount(imageHandle);
    }
  }
}

FramebufferHandle FramebufferAttachmentManager::storeFramebuffer(Framebuffer&& framebuffer, const FramebufferMetadata& metadata,
    std::span<const GpuImageHandle> attachments, VkImageView swapchainView) {
  FramebufferHandle handle = _freeFramebufferHandles.back();
  _freeFramebufferHandles.pop_back();

  if (swapchainView != VK_NULL_HANDLE) {
    _framebuffersSwapchainViews.emplace(framebuffer.getVkFramebuffer(), swapchainView);
  }
  for (GpuImageHandle imageHandle : attachments) {
    _gpuBufferManager.increaseRefCount(imageHandle);
  }
  _framebuffers.insertUnsafe(
      *handle, std::make_pair(std::move(framebuffer), FramebufferData{metadata, attachments}));

  return handle;
}

void FramebufferAttachmentManager::destroyFramebuffer(FramebufferHandle handle) {
  _freeFramebufferHandles.push_back(handle);
  _framebuffers.eraseUnsafe(*handle);
}

const Framebuffer& FramebufferAttachmentManager::getFramebuffer(FramebufferHandle handle) const {
  return _framebuffers.getValue(*handle).first;
}
