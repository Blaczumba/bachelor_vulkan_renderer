#include "index_buffer.h"

#include "buffers.h"
#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

IndexBuffer::IndexBuffer(const CommandPool& commandPool, const std::vector<uint8_t>& indices)
    : _logicalDevice(commandPool.getLogicalDevice()), _indexCount(indices.size()), _indexType(VK_INDEX_TYPE_UINT8_EXT) {
    createIndexBuffer(commandPool, indices.data(), sizeof(uint8_t) * _indexCount);
}

IndexBuffer::IndexBuffer(const CommandPool& commandPool, const std::vector<uint16_t>& indices)
    : _logicalDevice(commandPool.getLogicalDevice()), _indexCount(indices.size()), _indexType(VK_INDEX_TYPE_UINT16) {
    createIndexBuffer(commandPool, indices.data(), sizeof(uint16_t) * _indexCount);
}

IndexBuffer::IndexBuffer(const CommandPool& commandPool, const std::vector<uint32_t>& indices)
    : _logicalDevice(commandPool.getLogicalDevice()), _indexCount(indices.size()), _indexType(VK_INDEX_TYPE_UINT32) {
    createIndexBuffer(commandPool, indices.data(), sizeof(uint32_t) * _indexCount);
}

void IndexBuffer::createIndexBuffer(const CommandPool& commandPool, const void* indicesData, VkDeviceSize bufferSize) {
    const VkDevice device = _logicalDevice.getVkDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, indicesData, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);
    
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        copyBufferToBuffer(commandBuffer, stagingBuffer, _indexBuffer, bufferSize);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

IndexBuffer::~IndexBuffer() {
    const VkDevice device = _logicalDevice.getVkDevice();
    vkDestroyBuffer(device, _indexBuffer, nullptr);
    vkFreeMemory(device, _indexBufferMemory, nullptr);
}

const VkBuffer IndexBuffer::getVkBuffer() const {
    return _indexBuffer;
}

VkIndexType IndexBuffer::getIndexType() const {
    return _indexType;
}

uint32_t IndexBuffer::getIndexCount() const {
    return _indexCount;
}

void IndexBuffer::bind(const VkCommandBuffer commandBuffer) const {
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, _indexType);
}
