#pragma once

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <cstring>

class LogicalDevice;

template<typename IndexType>
class IndexBuffer {
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;
    uint32_t _indexCount;

    const LogicalDevice& _logicalDevice;
public:
    IndexBuffer(const LogicalDevice& logicalDevice, const std::vector<IndexType>& indices);
    ~IndexBuffer();

    VkIndexType getIndexType() const;
    const VkBuffer getVkBuffer() const;
    uint32_t getIndexCount() const;
};


template<typename IndexType>
IndexBuffer<IndexType>::IndexBuffer(const LogicalDevice& logicalDevice, const std::vector<IndexType>& indices)
    : _logicalDevice(logicalDevice) {
    _indexCount = indices.size();
    const VkDeviceSize bufferSize = sizeof(IndexType) * _indexCount;
    const VkDevice device = _logicalDevice.getVkDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);

    copyBuffer(_logicalDevice, stagingBuffer, _indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

template<typename IndexType>
IndexBuffer<IndexType>::~IndexBuffer() {
    const VkDevice device = _logicalDevice.getVkDevice();
    vkDestroyBuffer(device, _indexBuffer, nullptr);
    vkFreeMemory(device, _indexBufferMemory, nullptr);
}

template<typename IndexType>
const VkBuffer IndexBuffer<IndexType>::getVkBuffer() const {
    return _indexBuffer;
}

template<typename IndexType>
uint32_t IndexBuffer<IndexType>::getIndexCount() const {
    return _indexCount;
}
