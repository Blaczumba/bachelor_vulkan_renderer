#include "vulkan/wrapper/command_buffer/command_pool.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/command_buffer/command_buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/logical_device/resource_destroyer.h"
#include "vulkan/wrapper/util/check.h"

CommandPool::CommandPool(const LogicalDevice& logicalDevice, VkCommandPool commandPool) noexcept
  : _logicalDevice(logicalDevice), _commandPool(commandPool) {}

std::unique_ptr<CommandPool> CommandPool::create(
    const LogicalDevice& logicalDevice, VkCommandPoolCreateFlags flags) {
  const VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = flags,
    .queueFamilyIndex = *logicalDevice.getPhysicalDevice().getQueueFamilyIndices().graphicsFamily};

  VkCommandPool commandPool;
  CHECK_VKCMD(vkCreateCommandPool(logicalDevice.getVkDevice(), &poolInfo, nullptr, &commandPool),
              "Failed to create VkCommandPool in CommandPool::create.");
  return std::unique_ptr<CommandPool>(new CommandPool(logicalDevice, commandPool));
}

CommandPool::~CommandPool() {
  _logicalDevice.destroyResource([commandPool = _commandPool](DestroyerContext context) {
    vkDestroyCommandPool(context.device, commandPool, context.allocationCallbacks);
  });
}

CommandBuffer CommandPool::createCommandBuffer(VkCommandBufferLevel level) const {
  return CommandBuffer::create(shared_from_this(), level);
}

std::vector<CommandBuffer> CommandPool::createCommandBuffers(
    VkCommandBufferLevel level, uint32_t count) const {
  return CommandBuffer::create(shared_from_this(), level, count);
}

void CommandPool::reset() const {
  vkResetCommandPool(_logicalDevice.getVkDevice(), _commandPool, 0);
}

VkCommandPool CommandPool::getVkCommandPool() const noexcept {
  return _commandPool;
}

const LogicalDevice& CommandPool::getLogicalDevice() const {
  return _logicalDevice;
}
