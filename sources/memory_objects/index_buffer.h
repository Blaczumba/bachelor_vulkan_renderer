#pragma once

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <cstring>

class LogicalDevice;

class IndexBuffer {
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;
    const uint32_t _indexCount;
    const VkIndexType _indexType;

    const LogicalDevice& _logicalDevice;

public:
    IndexBuffer(const LogicalDevice& logicalDevice, const std::vector<uint8_t>& indices);
    IndexBuffer(const LogicalDevice& logicalDevice, const std::vector<uint16_t>& indices);
    IndexBuffer(const LogicalDevice& logicalDevice, const std::vector<uint32_t>& indices);
    ~IndexBuffer();

    VkIndexType getIndexType() const;
    const VkBuffer getVkBuffer() const;
    uint32_t getIndexCount() const;

private:
    void createIndexBuffer(const void* indicesData, VkDeviceSize bufferSize);
};
