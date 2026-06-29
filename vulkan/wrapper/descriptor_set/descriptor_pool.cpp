#include "descriptor_pool.h"

#include "common/util/engine_exception.h"
#include "descriptor_set.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

DescriptorPool::DescriptorPool(VkDescriptorPool descriptorPool, const LogicalDevice& logicalDevice,
                               uint32_t maxNumSets) noexcept
  : _descriptorPool(descriptorPool), _logicalDevice(logicalDevice), _maxNumSets(maxNumSets),
    _allocatedSets(0) {}

DescriptorPool::~DescriptorPool() {
  _logicalDevice.destroyResource([descriptorPool = _descriptorPool](DestroyerContext context) {
    vkDestroyDescriptorPool(context.device, descriptorPool, context.allocationCallbacks);
  });
}

std::unique_ptr<DescriptorPool> DescriptorPool::create(
    const LogicalDevice& logicalDevice, uint32_t maxNumSets, VkDescriptorPoolCreateFlags flags) {
  static constexpr VkDescriptorPoolSize poolSizes[] = {
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
  };

  const VkDescriptorPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = flags,
    .maxSets = maxNumSets,
    .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
    .pPoolSizes = poolSizes};

  VkDescriptorPool descriptorPool;
  CHECK_VKCMD(
      vkCreateDescriptorPool(logicalDevice.getVkDevice(), &poolInfo, nullptr, &descriptorPool),
      "Failed to create VkDescriptorPool.");
  return std::unique_ptr<DescriptorPool>(
      new DescriptorPool(descriptorPool, logicalDevice, maxNumSets));
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
  return nullptr;
}
