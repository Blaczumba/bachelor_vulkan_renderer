#include "vertex_buffer.h"

VertexBuffer::~VertexBuffer() {
    const VkDevice device = _logicalDevice.getVkDevice();
    vkDestroyBuffer(device, _vertexBuffer, nullptr);
    vkFreeMemory(device, _vertexBufferMemory, nullptr);
}

const VkBuffer VertexBuffer::getVkBuffer() const {
    return _vertexBuffer;
}

void VertexBuffer::bind(const VkCommandBuffer commandBuffer) const {
    const VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertexBuffer, offsets);
}
