#pragma once

#include "buffers.h"
#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#include <cstring>
#include <memory>
#include <vector>

class VertexBuffer {
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

	const LogicalDevice& _logicalDevice;

public:
    template<typename VertexType>
	VertexBuffer(const LogicalDevice& logicalDevice, const std::vector<VertexType>& vertices);
    ~VertexBuffer();

    const VkBuffer getVkBuffer() const;
};

template<typename VertexType>
VertexBuffer::VertexBuffer(const LogicalDevice& logicalDevice, const std::vector<VertexType>& vertices)
    : _logicalDevice(logicalDevice) {
    const VkDeviceSize bufferSize = sizeof(VertexType) * vertices.size();
    const VkDevice device = _logicalDevice.getVkDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
    
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        copyBufferToBuffer(commandBuffer, stagingBuffer, _vertexBuffer, bufferSize);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
