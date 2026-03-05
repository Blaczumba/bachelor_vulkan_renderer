#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

enum class AttachmentType : uint8_t {
  COLOR = 0,
  COLOR_RESOLVE,
  DEPTH,
  FRAGMENT_DENSITY_MAP
};

class AttachmentLayout {
public:
  explicit AttachmentLayout(VkSampleCountFlagBits numMsaaSamples = VK_SAMPLE_COUNT_1_BIT);

  std::span<const VkClearValue> getVkClearValues() const noexcept;

  std::span<const VkAttachmentDescription> getVkAttachmentDescriptions() const noexcept;

  size_t getAttachmentsCount() const noexcept;

  VkImageLayout getAttachmentVkImageLayout(uint32_t index) const;

  AttachmentType getAttachmentType(uint32_t index) const;

  uint32_t getColorAttachmentsCount() const noexcept;

  VkSampleCountFlagBits getNumMsaaSamples() const noexcept;

  AttachmentLayout& addColorAttachment(
      VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);

  AttachmentLayout& addColorPresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp);

  AttachmentLayout& addDepthAttachment(
      VkFormat format, VkAttachmentStoreOp storeOp,
      VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);

  AttachmentLayout& addShadowAttachment(VkFormat format, VkImageLayout finalLayout);

  AttachmentLayout& addColorResolveAttachment(
      VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);

  AttachmentLayout& addColorResolvePresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp);

  AttachmentLayout& addFragmentDensityMapAttachment();

private:
  VkSampleCountFlagBits _numMsaaSamples;
  std::vector<VkClearValue> _clearValues;
  std::vector<VkAttachmentDescription> _attachmentDescriptions;
  std::vector<VkImageLayout> _attachmentImageLayouts;
  std::vector<AttachmentType> _attachmentTypes;
};
