#include "vulkan/resource_manager/framebuffer_attachments_manager.h"

#include <numeric>
#include <ranges>
#include <span>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "common/util/resource_handles.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"

FramebufferAttachmentManager::FramebufferAttachmentManager(
    std::vector<FramebufferHandle>&& freeFramebuffers) noexcept
  : _freeFramebufferHandles(std::move(freeFramebuffers)) {}

std::unique_ptr<FramebufferAttachmentManager> FramebufferAttachmentManager::create() {
  std::vector<FramebufferHandle> freeFramebufferHandles(MAX_FRAMEBUFFERS);
  std::iota(freeFramebufferHandles.rbegin(), freeFramebufferHandles.rend(), FramebufferHandle(0));
  return std::unique_ptr<FramebufferAttachmentManager>(
      new FramebufferAttachmentManager(std::move(freeFramebufferHandles)));
}

FramebufferAttachmentManager::~FramebufferAttachmentManager() {}

FramebufferHandle FramebufferAttachmentManager::storeFramebuffer(
    Framebuffer&& framebuffer, const FramebufferMetadata& metadata,
    std::span<Ref<Image>> attachments, VkImageView swapchainView) {
  FramebufferHandle handle = _freeFramebufferHandles.back();
  _freeFramebufferHandles.pop_back();

  if (swapchainView != VK_NULL_HANDLE) {
    _framebuffersSwapchainViews.emplace(framebuffer.getVkFramebuffer(), swapchainView);
  }
  _framebuffers.insertUnsafe(
      *handle, std::make_pair(std::move(framebuffer),
                              FramebufferData{metadata, lib::Buffer<Ref<Image>>(attachments)}));

  return handle;
}

void FramebufferAttachmentManager::destroyFramebuffer(FramebufferHandle handle) {
  _freeFramebufferHandles.push_back(handle);
  _framebuffers.eraseUnsafe(*handle);
}

const Framebuffer& FramebufferAttachmentManager::getFramebuffer(FramebufferHandle handle) const {
  return _framebuffers.getValue(*handle).first;
}

const std::pair<Framebuffer, FramebufferData>&
FramebufferAttachmentManager::getFramebufferWithMetadata(FramebufferHandle handle) const {
  return _framebuffers.getValue(*handle);
}
