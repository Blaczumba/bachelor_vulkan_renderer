#pragma once

#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/render_pass/attachment_layout.h"

class Renderpass;

class Subpass {
public:
  Subpass(const AttachmentLayout& attachmentLayout) noexcept;

  ~Subpass() = default;

  Subpass& addOutputAttachment(uint32_t attachmentBinding);

  Subpass& addInputAttachment(uint32_t attachmentBinding);

  Subpass& withShadingRateAttachment();

  VkSubpassDescription2 getVkSubpassDescription(uint32_t viewMask = 0) const;

private:
  const AttachmentLayout& _attachmentLayout;

  void* _pNext = nullptr;

  std::vector<VkAttachmentReference2> _inputAttachmentRefs;
  std::vector<VkAttachmentReference2> _colorAttachmentRefs;
  std::vector<VkAttachmentReference2> _depthAttachmentRefs;
  std::vector<VkAttachmentReference2> _colorAttachmentResolveRefs;

  VkFragmentShadingRateAttachmentInfoKHR _shadingRateAttachmentInfo;
};

class RenderpassBuilder {
public:
  RenderpassBuilder(const AttachmentLayout& attachmentLayout);

  RenderpassBuilder& addDependency(
      uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask,
      VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask);

  Subpass& createSubpass();

  RenderpassBuilder& withMultiView(
      std::vector<uint32_t>&& viewMask, std::vector<uint32_t>&& correlationMask);

  Renderpass build(const LogicalDevice& logicalDevice);

private:
  const AttachmentLayout& _attachmentLayout;

  void* _pNext = nullptr;

  struct MultiViewInfo {
    VkRenderPassMultiviewCreateInfo multiviewCreateInfo;
    std::vector<uint32_t> viewMasks;
    std::vector<uint32_t> correlationMasks;
  };
  std::optional<MultiViewInfo> _multiViewInfo;
  VkRenderPassFragmentDensityMapCreateInfoEXT _fragmentDensityMapCreateInfo;

  std::vector<VkSubpassDependency2> _subpassDepencies;
  std::vector<Subpass> _subpasses;
};

class Renderpass {
public:
  Renderpass() noexcept = default;

  Renderpass(Renderpass&& renderpass) noexcept;

  Renderpass& operator=(Renderpass&& renderpass) noexcept;

  ~Renderpass();

  VkRenderPass getVkRenderPass() const noexcept;

  const AttachmentLayout& getAttachmentsLayout() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  Renderpass(const LogicalDevice& logicalDeivce, VkRenderPass renderpass,
             const AttachmentLayout& attachmentLayout) noexcept;

  void destroy();

  VkRenderPass _renderpass = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice = nullptr;
  AttachmentLayout _attachmentsLayout;

  friend class RenderpassBuilder;
};
