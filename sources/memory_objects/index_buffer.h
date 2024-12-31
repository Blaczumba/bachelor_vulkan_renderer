#pragma once

#include "memory_objects/staging_buffer.h"

#include <vulkan/vulkan.h>

#include <cstring>
#include <memory>
#include <vector>

class CommandPool;
class LogicalDevice;

class IndexBuffer {
    VkBuffer _indexBuffer;
    Allocation _allocation;
    const uint32_t _indexCount;
    const VkIndexType _indexType;

    const LogicalDevice& _logicalDevice;

public:
    IndexBuffer(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, const StagingBuffer& stagingBuffer, VkIndexType indexType, uint32_t indexCount);
    ~IndexBuffer();

    VkIndexType getIndexType() const;
    const VkBuffer getVkBuffer() const;
    uint32_t getIndexCount() const;
    void bind(const VkCommandBuffer commandBuffer) const;

private:
    struct Allocator {
        Allocation& allocation;
        const size_t size;

        const VkBuffer operator()(VmaWrapper& allocator) {
            auto [buffer, tmpAllocation, _] = allocator.createVkBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            allocation = tmpAllocation;
            return buffer;
        }

        const VkBuffer operator()(auto&&) {
            throw std::runtime_error("Unrecognized allocator in IndexBuffer creation");
        }
    };
};
