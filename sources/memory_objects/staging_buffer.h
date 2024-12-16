#pragma once

#include "memory_allocator/memory_allocator.h"
#include "memory_objects/buffers.h"
#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#include <span>
#include <variant>
#include <vector>

namespace {

class StagingBuffer {
public:
    template<typename Type>
    StagingBuffer(MemoryAllocator& memoryAllocator, const std::span<Type> data) : _memoryAllocator(memoryAllocator), _buffer(std::visit(Allocator{data.size() * sizeof(Type)}, memoryAllocator)) {
        std::memcpy(_buffer.data, data.data(), _buffer.size);
    }
    ~StagingBuffer() {
        std::visit(BufferDeallocator{ _buffer.buffer }, _memoryAllocator);
    }
    const Buffer& getBuffer() const {
        return _buffer;
    }

private:
    struct Allocator {
        const size_t size;

        Buffer operator()(VmaWrapper& wrapper) {
            const VkBuffer buffer = wrapper.createVkBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
            return Buffer{
                .buffer = buffer,
                .size = size,
                .data = wrapper.getMappedData(buffer)
            };
        }

        Buffer operator()(auto) {
            return Buffer{};
        }
    };

    const Buffer _buffer;
    MemoryAllocator& _memoryAllocator;
};

}


