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
	VertexBuffer(const CommandPool& commandPool, const std::vector<VertexType>& vertices);
    ~VertexBuffer();

    const VkBuffer getVkBuffer() const;
    void bind(const VkCommandBuffer commandBuffer) const;
};

template<typename VertexType>
VertexBuffer::VertexBuffer(const CommandPool& commandPool, const std::vector<VertexType>& vertices)
    : _logicalDevice(commandPool.getLogicalDevice()) {
    const VkDeviceSize bufferSize = sizeof(VertexType) * vertices.size();
    const VkDevice device = _logicalDevice.getVkDevice();

    const VkBuffer stagingBuffer = _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const VkDeviceMemory stagingBufferMemory = _logicalDevice.createBufferMemory(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    _vertexBuffer = _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    _vertexBufferMemory = _logicalDevice.createBufferMemory(_vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        copyBufferToBuffer(commandBuffer, stagingBuffer, _vertexBuffer, bufferSize);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
