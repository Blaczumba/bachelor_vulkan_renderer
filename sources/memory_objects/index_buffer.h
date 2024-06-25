#pragma once

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

template<typename IndexType>
class IndexBuffer {
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;

    std::shared_ptr<LogicalDevice> _logicalDevice;
public:
    IndexBuffer(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<IndexType>& indices);
    ~IndexBuffer();

    VkIndexType getIndexType() const;
    VkBuffer getVkBuffer() const;
};


template<typename IndexType>
IndexBuffer<IndexType>::IndexBuffer(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<IndexType>& indices)
    : _logicalDevice(logicalDevice) {
    VkDeviceSize bufferSize = sizeof(IndexType) * indices.size();
    VkDevice device = _logicalDevice->getVkDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    _logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);

    copyBuffer(_logicalDevice.get(), stagingBuffer, _indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

template<typename IndexType>
IndexBuffer<IndexType>::~IndexBuffer() {
    VkDevice device = _logicalDevice->getVkDevice();
    vkDestroyBuffer(device, _indexBuffer, nullptr);
    vkFreeMemory(device, _indexBufferMemory, nullptr);
}

template<typename IndexType>
VkBuffer IndexBuffer<IndexType>::getVkBuffer() const {
    return _indexBuffer;
}
