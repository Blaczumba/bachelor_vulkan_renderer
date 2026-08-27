#pragma once

#include <span>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/resource_handles.h"
#include "lib/buffer/buffer.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/ref.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/memory_objects/image.h"

struct FramebufferData {
  FramebufferMetadata metadata;
  lib::Buffer<Ref<Image>> imageRefs;
};

class FramebufferAttachmentManager {
  FramebufferAttachmentManager(std::vector<FramebufferHandle>&& freeFramebuffers) noexcept;

public:
  static std::unique_ptr<FramebufferAttachmentManager> create();

  ~FramebufferAttachmentManager();

  FramebufferHandle storeFramebuffer(
      Framebuffer&& framebuffer, const FramebufferMetadata& metadata,
      std::span<Ref<Image>> attachments, VkImageView swapchainView = VK_NULL_HANDLE);

  void destroyFramebuffer(FramebufferHandle handle);

  const Framebuffer& getFramebuffer(FramebufferHandle handle) const;

  const std::pair<Framebuffer, FramebufferData>& getFramebufferWithMetadata(
      FramebufferHandle handle) const;

private:
  lib::SparseMap<std::pair<Framebuffer, FramebufferData>, MAX_FRAMEBUFFERS> _framebuffers;
  std::vector<FramebufferHandle> _freeFramebufferHandles;

  std::unordered_map<VkFramebuffer, VkImageView> _framebuffersSwapchainViews;
};
