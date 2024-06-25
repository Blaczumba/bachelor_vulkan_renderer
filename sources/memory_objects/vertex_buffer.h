#pragma once

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

template<typename VertexType>
class VertexBuffer {
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	VertexBuffer(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<VertexType>& vertices);
    ~VertexBuffer();

    VkBuffer getVkBuffer() const;
};

template<typename VertexType>
VertexBuffer<VertexType>::VertexBuffer(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<VertexType>& vertices)
    : _logicalDevice(logicalDevice) {
    VkDeviceSize bufferSize = sizeof(VertexType) * vertices.size();
    VkDevice device = _logicalDevice->getVkDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    _logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);

    copyBuffer(_logicalDevice.get(), stagingBuffer, _vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

template<typename VertexType>
VertexBuffer<VertexType>::~VertexBuffer() {
    VkDevice device = _logicalDevice->getVkDevice();
    vkDestroyBuffer(device, _vertexBuffer, nullptr);
    vkFreeMemory(device, _vertexBufferMemory, nullptr);
}

template<typename VertexType>
VkBuffer VertexBuffer<VertexType>::getVkBuffer() const {
    return _vertexBuffer;
}