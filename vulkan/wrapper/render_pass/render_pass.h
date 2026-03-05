#pragma once

#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/render_pass/attachment_layout.h"

class Renderpass;

class RenderpassBuilder {
  class Subpass {
    std::vector<VkAttachmentReference> _inputAttachmentRefs;
    std::vector<VkAttachmentReference> _colorAttachmentRefs;
    std::vector<VkAttachmentReference> _depthAttachmentRefs;
    std::vector<VkAttachmentReference> _colorAttachmentResolveRefs;

  public:
    Subpass() noexcept = default;

    ~Subpass() = default;

    void addOutputAttachment(const AttachmentLayout& layout, uint32_t attachmentBinding);

    void addInputAttachment(
        const AttachmentLayout& layout, uint32_t attachmentBinding, VkImageLayout imageLayout);

    VkSubpassDescription getVkSubpassDescription() const;
  };

public:
  RenderpassBuilder(const AttachmentLayout& attachmentLayout);

  RenderpassBuilder& addDependency(
      uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask,
      VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask);

  RenderpassBuilder& addSubpass(std::initializer_list<uint8_t> outputAttachments,
                                std::initializer_list<uint8_t> inputAttachments = {});

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

  std::vector<VkSubpassDependency> _subpassDepencies;
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
