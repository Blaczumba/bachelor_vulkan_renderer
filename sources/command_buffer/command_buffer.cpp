#include "logical_device/logical_device.h"
#include "command_buffer.h"

#include <stdexcept>

class LogicalDevice;

SingleTimeCommandBuffer::SingleTimeCommandBuffer(const LogicalDevice& logicalDevice)
    : _logicalDevice(logicalDevice) {

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateFence(_logicalDevice.getVkDevice(), &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create SingleTimeCommandBuffer fence!");
    }

    _commandBuffer = _logicalDevice.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
    const VkDevice device = _logicalDevice._device;

    vkEndCommandBuffer(_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &_commandBuffer;

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

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}