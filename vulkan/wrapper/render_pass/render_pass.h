#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/render_pass/attachment_layout.h"

class Renderpass {
  Renderpass(const LogicalDevice& logicalDeivce, VkRenderPass renderpass,
             const AttachmentLayout& attachmentLayout) noexcept;

public:
  Renderpass() noexcept = default;

  Renderpass(Renderpass&& renderpass) noexcept;

  Renderpass& operator=(Renderpass&& renderpass) noexcept;

  ~Renderpass();

  static Renderpass create(
      const LogicalDevice& logicalDevice, const VkRenderPassCreateInfo2& createInfo);

  VkRenderPass getVkRenderPass() const noexcept;

  const AttachmentLayout& getAttachmentsLayout() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  void destroy();

  VkRenderPass _renderpass = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice = nullptr;
  AttachmentLayout _attachmentsLayout;

  friend class RenderpassBuilder;
};

class RenderpassBuilder {
  class Subpass {
  public:
    Subpass(const AttachmentLayout& attachmentLayout) noexcept;

    ~Subpass() = default;

    Subpass& addOutputAttachment(uint32_t attachmentBinding);

    Subpass& addInputAttachment(uint32_t attachmentBinding);

    Subpass& withShadingRateAttachment(uint32_t texelWidth, uint32_t texelHeight);

    Subpass& withShadingRateAttachment(uint32_t binding, uint32_t texelWidth, uint32_t texelHeight);

    VkSubpassDescription2 getVkSubpassDescription(uint32_t viewMask = 0) const;

  private:
    const AttachmentLayout& _attachmentLayout;

    void* _pNext = nullptr;

    std::vector<VkAttachmentReference2> _inputAttachmentRefs;
    std::vector<VkAttachmentReference2> _colorAttachmentRefs;
    std::vector<VkAttachmentReference2> _depthAttachmentRefs;
    std::vector<VkAttachmentReference2> _colorAttachmentResolveRefs;
    std::optional<VkAttachmentReference2> _fragmentShadingRateAttachmentRef;

    VkFragmentShadingRateAttachmentInfoKHR _shadingRateAttachmentInfo;
  };

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
    std::vector<uint32_t> viewMasks;
    std::vector<uint32_t> correlationMasks;
  };
  std::optional<MultiViewInfo> _multiViewInfo;
  VkRenderPassFragmentDensityMapCreateInfoEXT _fragmentDensityMapCreateInfo;

  std::vector<VkSubpassDependency2> _subpassDepencies;
  // For reference stability use unique_ptr.
  std::vector<std::unique_ptr<Subpass>> _subpasses;
};
