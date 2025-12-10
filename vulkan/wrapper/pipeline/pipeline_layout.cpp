#include "pipeline_layout.h"

#include <utility>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

PipelineLayout::PipelineLayout(const LogicalDevice& logicalDevice, VkPipelineLayout layout)
	: _logicalDevice(&logicalDevice), _layout(layout) {}

ErrorOr<PipelineLayout> PipelineLayout::create(
    const LogicalDevice& logicalDevice,
    std::span<const VkDescriptorSetLayout> descriptorSetLayouts,
    std::span<const VkPushConstantRange> pushConstantRanges,
    VkPipelineLayoutCreateFlags flags) {
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .flags = flags,
    .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
    .pSetLayouts = descriptorSetLayouts.data(),
    .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
    .pPushConstantRanges = pushConstantRanges.data(),
  };

  VkPipelineLayout layout;
  CHECK_VKCMD(
      vkCreatePipelineLayout(logicalDevice.getVkDevice(), &pipelineLayoutInfo, nullptr, &layout));
  return PipelineLayout(logicalDevice, layout);
}

ErrorOr<PipelineLayout> PipelineLayout::wrap(
    const LogicalDevice& logicalDevice, VkPipelineLayout layout) {
  if (layout == VK_NULL_HANDLE) {
    return Error(EngineError::NULLPTR_REFERENCE);
  }

  return PipelineLayout(logicalDevice, layout);
}

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
  : _logicalDevice(other._logicalDevice), _layout(std::exchange(other._layout, VK_NULL_HANDLE)) {}

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  _logicalDevice = other._logicalDevice;
  _layout = std::exchange(other._layout, VK_NULL_HANDLE);
  return *this;
}

PipelineLayout ::~PipelineLayout() {
  if (_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(_logicalDevice->getVkDevice(), _layout, nullptr);
  }
}

VkPipelineLayout PipelineLayout::getVkPipelineLayout() const {
  return _layout;
}

const LogicalDevice& PipelineLayout::getLogicalDevice() const {
  return *_logicalDevice;
}
