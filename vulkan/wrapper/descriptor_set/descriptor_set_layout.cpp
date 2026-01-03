#include "descriptor_set_layout.h"

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

DescriptorSetLayout::DescriptorSetLayout(
    const LogicalDevice& logicalDevice, VkDescriptorSetLayout layout) noexcept
  : _logicalDevice(&logicalDevice), _descriptorSetLayout(layout) {}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& layout) noexcept
  : _descriptorSetLayout(std::exchange(layout._descriptorSetLayout, VK_NULL_HANDLE)),
    _logicalDevice(layout._logicalDevice) {}

void DescriptorSetLayout::destroy() {
  if (_descriptorSetLayout != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource(
        [descriptorSetlayout = _descriptorSetLayout](DestroyerContext context) {
          vkDestroyDescriptorSetLayout(
              context.device, descriptorSetlayout, context.allocationCallbacks);
        });
  }
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& layout) noexcept {
  if (&layout == this) {
    return *this;
  }

  destroy();

  _descriptorSetLayout = std::exchange(layout._descriptorSetLayout, VK_NULL_HANDLE);
  _logicalDevice = std::exchange(layout._logicalDevice, nullptr);
  return *this;
}

DescriptorSetLayout::~DescriptorSetLayout() {
  destroy();
}

DescriptorSetLayout DescriptorSetLayout::create(
    const LogicalDevice& logicalDevice, std::span<const VkDescriptorSetLayoutBinding> bindings,
    std::span<const VkDescriptorBindingFlags> bindFlags, VkDescriptorSetLayoutCreateFlags flags) {
  const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
    .bindingCount = static_cast<uint32_t>(bindFlags.size()),
    .pBindingFlags = (!bindFlags.empty()) ? bindFlags.data() : nullptr};

  const VkDescriptorSetLayoutCreateInfo layoutInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext = &bindingFlags,
    .flags = flags,
    .bindingCount = static_cast<uint32_t>(bindings.size()),
    .pBindings = bindings.data()};

  VkDescriptorSetLayout descriptorSetLayout;
  CHECK_VKCMD(vkCreateDescriptorSetLayout(
                  logicalDevice.getVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout),
              "Failed to create VkDescriptorSetLayout.");
  return DescriptorSetLayout(logicalDevice, descriptorSetLayout);
}

VkDescriptorSetLayout DescriptorSetLayout::getVkDescriptorSetLayout() const {
  return _descriptorSetLayout;
}
