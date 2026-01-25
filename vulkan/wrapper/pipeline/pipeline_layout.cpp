#include "pipeline_layout.h"

#include <utility>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

PipelineLayout::PipelineLayout(const LogicalDevice& logicalDevice, VkPipelineLayout layout) noexcept
  : _logicalDevice(&logicalDevice), _layout(layout) {}

PipelineLayout PipelineLayout::create(
    const LogicalDevice& logicalDevice, std::span<const VkDescriptorSetLayout> descriptorSetLayouts,
    std::span<const VkPushConstantRange> pushConstantRanges, VkPipelineLayoutCreateFlags flags) {
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
      vkCreatePipelineLayout(logicalDevice.getVkDevice(), &pipelineLayoutInfo, nullptr, &layout),
      "Failed to create VkPipelineLayout.");
  return PipelineLayout(logicalDevice, layout);
}

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
  : _logicalDevice(std::exchange(other._logicalDevice, nullptr)),
    _layout(std::exchange(other._layout, VK_NULL_HANDLE)) {}

void PipelineLayout::destroy() {
  if (_layout != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([layout = _layout](DestroyerContext context) {
      vkDestroyPipelineLayout(context.device, layout, context.allocationCallbacks);
    });
  }
}

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  destroy();

  _logicalDevice = std::exchange(other._logicalDevice, nullptr);
  _layout = std::exchange(other._layout, VK_NULL_HANDLE);
  return *this;
}

PipelineLayout::~PipelineLayout() {
  destroy();
}

VkPipelineLayout PipelineLayout::getVkPipelineLayout() const noexcept {
  return _layout;
}

const LogicalDevice& PipelineLayout::getLogicalDevice() const {
  return *_logicalDevice;
}
