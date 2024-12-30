#pragma once

#include "memory_allocator/memory_allocator.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

struct Buffer {
    VkBuffer buffer;
    size_t size;
    void* data;
};

struct BufferDeallocator {
    const VkBuffer buffer;

    void operator()(VmaWrapper& allocator, const VmaAllocation allocation);
    void operator()(auto&&, auto&&) {
        throw std::runtime_error("BufferDeallocator does not support such MemoryAllocator type!");
    }
};