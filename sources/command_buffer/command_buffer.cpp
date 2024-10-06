#include "logical_device/logical_device.h"
#include "command_buffer.h"

#include <stdexcept>

CommandPool::CommandPool(const LogicalDevice& logicalDevice) : _logicalDevice(logicalDevice) {
    const VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _logicalDevice.getPhysicalDevice().getPropertyManager().getQueueFamilyIndices().graphicsFamily.value()
    };

    if (vkCreateCommandPool(_logicalDevice.getVkDevice(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(_logicalDevice.getVkDevice(), _commandPool, nullptr);
}
std::unique_ptr<CommandBuffer> CommandPool::createPrimaryCommandBuffer() const {
    return std::make_unique<CommandBuffer>(*this, true);
}

std::unique_ptr<CommandBuffer> CommandPool::createSecondaryCommandBuffer() const {
    return std::make_unique<CommandBuffer>(*this, false);
}

const VkCommandPool CommandPool::getVkCommandPool() const {
    return _commandPool;
}

const LogicalDevice& CommandPool::getLogicalDevice() const {
    return _logicalDevice;
}

CommandBuffer::CommandBuffer(const CommandPool& commandPool, bool primary) :_commandPool(commandPool) {
    VkCommandBufferLevel level = (primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool.getVkCommandPool(),
        .level = level,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(_commandPool.getLogicalDevice().getVkDevice(), &allocInfo, &_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandBuffer::resetCommandBuffer() const {
    vkResetCommandBuffer(_commandBuffer, 0);
}

const VkCommandBuffer CommandBuffer::getVkCommandBuffer() const {
    return _commandBuffer;
}

SingleTimeCommandBuffer::SingleTimeCommandBuffer(const LogicalDevice& logicalDevice)
    : _logicalDevice(logicalDevice) {

    const VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };

    if (vkCreateFence(_logicalDevice.getVkDevice(), &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create SingleTimeCommandBuffer fence!");
    }

    _commandBuffer = _logicalDevice.createCommandBuffer();

    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
    const VkDevice device = _logicalDevice._device;

    vkEndCommandBuffer(_commandBuffer);

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &_commandBuffer
    };

    vkQueueSubmit(_logicalDevice.graphicsQueue, 1, &submitInfo, _fence);
    vkWaitForFences(device, 1, &_fence, VK_TRUE, UINT64_MAX);

    vkFreeCommandBuffers(device, _logicalDevice._commandPool, 1, &_commandBuffer);
    vkDestroyFence(device, _fence, nullptr);
}

VkCommandBuffer SingleTimeCommandBuffer::getCommandBuffer() const {
    return _commandBuffer;
}

void copyBuffer(const LogicalDevice& logicalDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    SingleTimeCommandBuffer handle(logicalDevice);
    VkCommandBuffer commandBuffer = handle.getCommandBuffer();

    const VkBufferCopy copyRegion = {
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}