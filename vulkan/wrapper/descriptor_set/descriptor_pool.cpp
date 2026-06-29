#include "descriptor_pool.h"

#include <cstdint>
#include <span>

#include "common/util/engine_exception.h"
#include "descriptor_set.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

DescriptorPool::DescriptorPool(
    const LogicalDevice& logicalDevice, VkDescriptorPool descriptorPool, uint32_t maxNumSets,
    std::span<const VkDescriptorPoolSize> poolSizes) noexcept
  : _logicalDevice(logicalDevice), _descriptorPool(descriptorPool), _maxNumSets(maxNumSets),
    _allocatedSets(0), _poolSizes(poolSizes) {}

DescriptorPool::~DescriptorPool() {
  _logicalDevice.destroyResource([descriptorPool = _descriptorPool](DestroyerContext context) {
    vkDestroyDescriptorPool(context.device, descriptorPool, context.allocationCallbacks);
  });
}

std::unique_ptr<DescriptorPool> DescriptorPool::create(
    const LogicalDevice& logicalDevice, const VkDescriptorPoolCreateInfo& createInfo) {
  VkDescriptorPool descriptorPool;
  CHECK_VKCMD(
      vkCreateDescriptorPool(logicalDevice.getVkDevice(), &createInfo, nullptr, &descriptorPool),
      "Failed to create VkDescriptorPool.");
  return std::unique_ptr<DescriptorPool>(new DescriptorPool(
      logicalDevice, descriptorPool, createInfo.maxSets,
      std::span<const VkDescriptorPoolSize>(createInfo.pPoolSizes, createInfo.poolSizeCount)));
}

VkDescriptorPool DescriptorPool::getVkDescriptorPool() const noexcept {
  return _descriptorPool;
}

DescriptorSet DescriptorPool::createDesriptorSet(VkDescriptorSetLayout layout) const {
  ++_allocatedSets;
  if (_allocatedSets > _maxNumSets) {
    --_allocatedSets;
    throw EngineException("Cannot allocate more descriptor sets from the descriptor set pool.");
  }
  return DescriptorSet::create(shared_from_this(), layout);
}

bool DescriptorPool::maxSetsReached() const noexcept {
  return _allocatedSets >= _maxNumSets;
}

const LogicalDevice& DescriptorPool::getLogicalDevice() const {
  return _logicalDevice;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::addPoolSize(
    VkDescriptorType type, uint32_t descriptorCount) {
  _poolSizes.emplace_back(type, descriptorCount);
  return *this;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::withPoolSizes(
    std::span<const VkDescriptorPoolSize> poolSizes) {
  _poolSizes.assign_range(poolSizes);
  return *this;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::withPoolSizes(
    std::initializer_list<VkDescriptorPoolSize> poolSizes) {
  _poolSizes.assign_range(poolSizes);
  return *this;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::withPoolSizes(
    std::vector<VkDescriptorPoolSize>&& poolSizes) noexcept {
  _poolSizes = std::move(poolSizes);
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPoolBuilder::build(
    const LogicalDevice& logicalDevice, uint32_t maxNumSets, VkDescriptorPoolCreateFlags flags) {
  const VkDescriptorPoolCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = _flags = flags,
    .maxSets = _maxNumSets = maxNumSets,
    .poolSizeCount = static_cast<uint32_t>(_poolSizes.size()),
    .pPoolSizes = !_poolSizes.empty() ? _poolSizes.data() : nullptr};
  return DescriptorPool::create(logicalDevice, createInfo);
}
