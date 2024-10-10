#pragma once

#include <vulkan/vulkan.h>

#include <cstring>
#include <memory>
#include <vector>

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
