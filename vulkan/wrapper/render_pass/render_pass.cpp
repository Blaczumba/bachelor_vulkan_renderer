#include "render_pass.h"

#include <algorithm>
#include <iterator>

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/render_pass/attachment_layout.h"
#include "vulkan/wrapper/util/check.h"

namespace {

template <typename T>
void chainExtendedField(void** next, T& feature) {
  feature.pNext = *next;
  *next = (void*)&feature;
}

}  // namespace

RenderpassBuilder::RenderpassBuilder(const AttachmentLayout& attachmentLayout)
  : _attachmentLayout(attachmentLayout) {}

RenderpassBuilder& RenderpassBuilder::addDependency(
    uint32_t srcSubpassIndex, uint32_t dstSubpassIndex, VkPipelineStageFlags srcStageMask,
    VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask) {
  _subpassDepencies.push_back(VkSubpassDependency2{
    .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
    .srcSubpass = srcSubpassIndex,
    .dstSubpass = dstSubpassIndex,
    .srcStageMask = srcStageMask,
    .dstStageMask = dstStageMask,
    .srcAccessMask = srcAccessMask,
    .dstAccessMask = dstAccessMask});
  return *this;
}

Subpass& RenderpassBuilder::createSubpass() {
  return _subpasses.emplace_back(_attachmentLayout);
}

RenderpassBuilder& RenderpassBuilder::withMultiView(
    std::vector<uint32_t>&& viewMask, std::vector<uint32_t>&& correlationMask) {
  _multiViewInfo = MultiViewInfo{
    .multiviewCreateInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
                            .pViewMasks = viewMask.data(),
                            .correlationMaskCount = static_cast<uint32_t>(correlationMask.size()),
                            .pCorrelationMasks = correlationMask.data()},
    .viewMasks = std::move(viewMask),
    .correlationMasks = std::move(correlationMask)
  };

  // vkCreateRenderPass2KHR does not extend this structure.
  // chainExtendedField(&_pNext, _multiViewInfo->multiviewCreateInfo);
  return *this;
}

Subpass::Subpass(const AttachmentLayout& attachmentLayout) noexcept
  : _attachmentLayout(attachmentLayout) {}

Subpass& Subpass::addOutputAttachment(uint32_t attachmentBinding) {
  const VkAttachmentReference2 attachmentRef{
    .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
    .attachment = attachmentBinding,
    .layout = _attachmentLayout.getAttachmentVkImageLayout(attachmentBinding),
    .aspectMask = _attachmentLayout.getAttachmentAspectFlags(attachmentBinding)};
  switch (_attachmentLayout.getAttachmentType(attachmentBinding)) {
    case AttachmentType::COLOR:
      _colorAttachmentRefs.push_back(attachmentRef);
      break;
    case AttachmentType::COLOR_RESOLVE:
      _colorAttachmentResolveRefs.push_back(attachmentRef);
      break;
    case AttachmentType::DEPTH:
      _depthAttachmentRefs.push_back(attachmentRef);
      break;
    default:
      throw EngineException("Failed to recognize attachment type.");
  }
  return *this;
}

Subpass& Subpass::addInputAttachment(uint32_t attachmentBinding) {
  _inputAttachmentRefs.push_back(VkAttachmentReference2{
    .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
    .attachment = attachmentBinding,
    .layout = _attachmentLayout.getAttachmentVkImageLayout(attachmentBinding),
    .aspectMask = _attachmentLayout.getAttachmentAspectFlags(attachmentBinding)});
  return *this;
}

Subpass& Subpass::withShadingRateAttachment() {
  _shadingRateAttachmentInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR,
  };

  chainExtendedField(&_pNext, _shadingRateAttachmentInfo);
  return *this;
}

VkSubpassDescription2 Subpass::getVkSubpassDescription(uint32_t viewMask) const {
  return VkSubpassDescription2{
    .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
    .pNext = _pNext,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .viewMask = viewMask,
    .inputAttachmentCount = static_cast<uint32_t>(_inputAttachmentRefs.size()),
    .pInputAttachments = !_inputAttachmentRefs.empty() ? _inputAttachmentRefs.data() : nullptr,
    .colorAttachmentCount = static_cast<uint32_t>(_colorAttachmentRefs.size()),
    .pColorAttachments = !_colorAttachmentRefs.empty() ? _colorAttachmentRefs.data() : nullptr,
    .pResolveAttachments =
        !_colorAttachmentResolveRefs.empty() ? _colorAttachmentResolveRefs.data() : nullptr,
    .pDepthStencilAttachment =
        !_depthAttachmentRefs.empty() ? _depthAttachmentRefs.data() : nullptr};
}

Renderpass RenderpassBuilder::build(const LogicalDevice& logicalDevice) {
  std::span<const VkAttachmentDescription2> attachmentDescriptions =
      _attachmentLayout.getVkAttachmentDescriptions();
  lib::Buffer<VkSubpassDescription2> subpassDescriptions(_subpasses.size());
  for (int i = 0; i < _subpasses.size(); i++) {
    subpassDescriptions[i] = _subpasses[i].getVkSubpassDescription(
        _multiViewInfo.has_value() ? _multiViewInfo->viewMasks[i] : 0);
  }

  if (_multiViewInfo.has_value()) {
    _multiViewInfo->multiviewCreateInfo.subpassCount =
        static_cast<uint32_t>(subpassDescriptions.size());
  }

  for (uint32_t i = 0; i < _attachmentLayout.getAttachmentsCount(); i++) {
    if (_attachmentLayout.getAttachmentType(i) == AttachmentType::FRAGMENT_DENSITY_MAP) {
      _fragmentDensityMapCreateInfo = VkRenderPassFragmentDensityMapCreateInfoEXT{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
        .fragmentDensityMapAttachment =
            VkAttachmentReference{i, _attachmentLayout.getAttachmentVkImageLayout(i)}
      };
      chainExtendedField(&_pNext, _fragmentDensityMapCreateInfo);
      break;
    }
  }

  const VkRenderPassCreateInfo2 renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
    .pNext = _pNext,
    .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
    .pAttachments = !attachmentDescriptions.empty() ? attachmentDescriptions.data() : nullptr,
    .subpassCount = static_cast<uint32_t>(subpassDescriptions.size()),
    .pSubpasses = !subpassDescriptions.empty() ? subpassDescriptions.data() : nullptr,
    .dependencyCount = static_cast<uint32_t>(_subpassDepencies.size()),
    .pDependencies = !_subpassDepencies.empty() ? _subpassDepencies.data() : nullptr,
    .correlatedViewMaskCount = static_cast<uint32_t>(
        _multiViewInfo.has_value() ? _multiViewInfo->correlationMasks.size() : 0),
    .pCorrelatedViewMasks =
        _multiViewInfo.has_value() ? _multiViewInfo->correlationMasks.data() : nullptr};

  VkRenderPass renderpass;
  CHECK_VKCMD(
      vkCreateRenderPass2(logicalDevice.getVkDevice(), &renderPassInfo, nullptr, &renderpass),
      "Failed to create VkRenderPass.");
  return Renderpass(logicalDevice, renderpass, _attachmentLayout);
}

Renderpass::Renderpass(const LogicalDevice& logicalDeivce, VkRenderPass renderpass,
                       const AttachmentLayout& attachmentLayout) noexcept
  : _logicalDevice(&logicalDeivce), _renderpass(renderpass), _attachmentsLayout(attachmentLayout) {}

Renderpass::Renderpass(Renderpass&& renderpass) noexcept
  : _renderpass(std::exchange(renderpass._renderpass, VK_NULL_HANDLE)),
    _logicalDevice(std::exchange(renderpass._logicalDevice, nullptr)),
    _attachmentsLayout(std::move(renderpass._attachmentsLayout)) {}

void Renderpass::destroy() {
  if (_renderpass != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([renderpass = _renderpass](DestroyerContext context) {
      vkDestroyRenderPass(context.device, renderpass, context.allocationCallbacks);
    });
  }
}

Renderpass& Renderpass::operator=(Renderpass&& renderpass) noexcept {
  if (this == &renderpass) [[unlikely]] {
    return *this;
  }

  destroy();

  _renderpass = std::exchange(renderpass._renderpass, VK_NULL_HANDLE);
  _logicalDevice = std::exchange(renderpass._logicalDevice, nullptr);
  _attachmentsLayout = std::move(renderpass._attachmentsLayout);
  return *this;
}

Renderpass::~Renderpass() {
  destroy();
}

VkRenderPass Renderpass::getVkRenderPass() const noexcept {
  return _renderpass;
}

const AttachmentLayout& Renderpass::getAttachmentsLayout() const noexcept {
  return _attachmentsLayout;
}

const LogicalDevice& Renderpass::getLogicalDevice() const {
  return *_logicalDevice;
}
