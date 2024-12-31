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
    StagingBuffer(MemoryAllocator& memoryAllocator, const std::span<Type> data) : _memoryAllocator(memoryAllocator) {
        _buffer = std::visit(Allocator{ _allocation, data.size() * sizeof(Type) }, memoryAllocator);
        std::memcpy(_buffer.data, data.data(), _buffer.size);
    }

    ~StagingBuffer() {
        if (_buffer.buffer != VK_NULL_HANDLE) {
            std::visit(BufferDeallocator{ _buffer.buffer }, _memoryAllocator, _allocation);
        }
    }

    StagingBuffer(StagingBuffer&& stagingBuffer) noexcept
        : _buffer(stagingBuffer._buffer),
        _memoryAllocator(stagingBuffer._memoryAllocator) {
        stagingBuffer._buffer.buffer = VK_NULL_HANDLE;
    }

    const Buffer& getBuffer() const {
        return _buffer;
    }

    const VkBuffer getVkBuffer() const {
        return _buffer.buffer;
    }

    size_t getSize() const {
        return _buffer.size;
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
    MemoryAllocator& _memoryAllocator;
};


