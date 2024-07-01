#include "command_buffer.h"

SingleTimeCommandBuffer::SingleTimeCommandBuffer(LogicalDevice* logicalDevice)
    : _logicalDevice(logicalDevice) {

    _commandBuffer = _logicalDevice->createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
    vkEndCommandBuffer(_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    vkQueueSubmit(_logicalDevice->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_logicalDevice->graphicsQueue);

    vkFreeCommandBuffers(_logicalDevice->_device, _logicalDevice->_commandPool, 1, &_commandBuffer);
}

VkCommandBuffer SingleTimeCommandBuffer::getCommandBuffer() const {
    return _commandBuffer;
}

void copyBuffer(LogicalDevice* logicalDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    SingleTimeCommandBuffer handle(logicalDevice);
    VkCommandBuffer commandBuffer = handle.getCommandBuffer();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}