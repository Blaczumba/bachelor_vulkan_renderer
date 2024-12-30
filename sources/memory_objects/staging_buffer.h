#pragma once

#include "memory_allocator/memory_allocator.h"
#include "memory_objects/buffer.h"
#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#include <span>
#include <variant>
#include <vector>
#include <iostream>


class StagingBuffer {
public:
    template<typename Type>
    StagingBuffer(MemoryAllocator& memoryAllocator, const std::span<Type> data, const std::vector<VkBufferImageCopy>& copyRegions = {}) : _memoryAllocator(memoryAllocator), _copyRegions(copyRegions) {
        _buffer = std::visit(Allocator{ _allocation, data.size() * sizeof(Type) }, memoryAllocator);
        std::memcpy(_buffer.data, data.data(), _buffer.size);
    }
    ~StagingBuffer() {
        if (_buffer.buffer) {
            std::visit(BufferDeallocator{ _buffer.buffer }, _memoryAllocator, _allocation);
        }
    }
    StagingBuffer(StagingBuffer&& stagingBuffer)
        : _buffer(stagingBuffer._buffer),
        _copyRegions(std::move(stagingBuffer._copyRegions)),
        _memoryAllocator(stagingBuffer._memoryAllocator) {
        stagingBuffer._buffer.buffer = VK_NULL_HANDLE;
    }
    const Buffer& getBuffer() const {
        return _buffer;
    }
    const std::vector<VkBufferImageCopy>& getImageCopyRegions() const {
        return _copyRegions;
    }

private:
    struct Allocator {
        Allocation& allocation;
        const size_t size;

        Buffer operator()(VmaWrapper& wrapper) {
            const auto[buffer, tmpallocation, data] = wrapper.createVkBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
            allocation = tmpallocation;
            return Buffer{
                .buffer = buffer,
                .size = size,
                .data = data
            };
        }

        Buffer operator()(auto) {
            return Buffer{};
        }
    };
    Allocation _allocation;
    Buffer _buffer;
    std::vector<VkBufferImageCopy> _copyRegions;
    MemoryAllocator& _memoryAllocator;
};


