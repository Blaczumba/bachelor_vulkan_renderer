#include "buffer.h"

#include <optional>
#include <span>
#include <variant>
#include <vulkan/vulkan.h>

std::span<const std::byte> BufferMetadata::getMappedMemoryAsSpan() const noexcept {
  return std::span(mappedMemory, size);
}

std::span<std::byte> BufferMetadata::getMappedMemoryAsSpan() noexcept {
  return std::span(mappedMemory, size);
}

Buffer::Buffer(const LogicalDevice& logicalDevice, VkBuffer buffer, Allocation allocation) noexcept
  : _logicalDevice(&logicalDevice), _buffer(buffer), _allocation(allocation) {}

Buffer::Buffer(Buffer&& buffer) noexcept
  : _buffer(std::exchange(buffer._buffer, VK_NULL_HANDLE)), _allocation(buffer._allocation),
    _logicalDevice(std::exchange(buffer._logicalDevice, nullptr)) {}

Buffer& Buffer::operator=(Buffer&& buffer) noexcept {
  if (this == &buffer) {
    return *this;
  }

  if (_buffer != VK_NULL_HANDLE) {
    destroy();
  }

  _buffer = std::exchange(buffer._buffer, VK_NULL_HANDLE);
  _allocation = buffer._allocation;
  _logicalDevice = std::exchange(buffer._logicalDevice, nullptr);
  return *this;
}

namespace {

struct BufferDeallocator {
  const VkBuffer buffer;

  void operator()(VmaWrapper& allocator, const VmaAllocation allocation) {
    allocator.destroyVkBuffer(buffer, allocation);
  }

  void operator()(auto&&, auto&&) {}
};

}  // namespace

void Buffer::destroy() {
  _logicalDevice->destroyResource(
      [buffer = _buffer, allocation = _allocation](DestroyerContext context) {
        std::visit(BufferDeallocator{buffer}, *context.memoryAllocator, allocation);
      });
}

Buffer::~Buffer() {
  if (_buffer != VK_NULL_HANDLE) {
    destroy();
    _buffer = VK_NULL_HANDLE;
  }
}

namespace {

struct BufferResources {
  VkBuffer buffer;
  Allocation allocation;
  void* mappedMemory;
};

struct VertexInputBufferAllocator {
  const VkBufferCreateInfo& bufferCreateInfo;

  BufferResources operator()(VmaWrapper& allocator) {
    const VmaWrapper::Buffer buffer =
        allocator.createVkBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    return BufferResources{buffer.buffer, buffer.allocation};
  }

  BufferResources operator()(auto&&) {
    return {};
  }
};

struct StagingBufferAllocator {
  const VkBufferCreateInfo& bufferCreateInfo;

  BufferResources operator()(VmaWrapper& wrapper) {
    const VmaWrapper::Buffer buffer = wrapper.createVkBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    return BufferResources{buffer.buffer, buffer.allocation, buffer.mappedData};
  }

  BufferResources operator()(auto&&) {
    return {};
  }
};

struct UniformBufferAllocator {
  const VkBufferCreateInfo& bufferCreateInfo;

  BufferResources operator()(VmaWrapper& allocator) {
    const VmaWrapper::Buffer buffer = allocator.createVkBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    return BufferResources{buffer.buffer, buffer.allocation, buffer.mappedData};
  }

  BufferResources operator()(auto&&) {
    return {};
  }
};

template <typename Allocator>
BufferResources createBuffer(const LogicalDevice& logicalDevice, const BufferMetadata& metadata) {
  const VkBufferCreateInfo bufferCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = metadata.flags,
    .size = metadata.size,
    .usage = metadata.usage,
    .sharingMode = metadata.sharingMode,
    .queueFamilyIndexCount = static_cast<uint32_t>(metadata.queueFamilyIndices.size()),
    .pQueueFamilyIndices =
        metadata.queueFamilyIndices.empty() ? nullptr : metadata.queueFamilyIndices.data(),
  };
  return std::visit(Allocator{bufferCreateInfo}, logicalDevice.getMemoryAllocator());
}

}  // namespace

Buffer BufferBuilder::createVertexInputBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<VertexInputBufferAllocator>(logicalDevice, _metadata);
  _metadata.mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}

Buffer BufferBuilder::createStagingBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<StagingBufferAllocator>(logicalDevice, _metadata);
  _metadata.mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}

Buffer BufferBuilder::createUniformBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<UniformBufferAllocator>(logicalDevice, _metadata);
  _metadata.mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}

const VkBuffer& Buffer::getVkBuffer() const noexcept {
  return _buffer;
}

const LogicalDevice& Buffer::getLogicalDevice() const noexcept {
  return *_logicalDevice;
}

BufferBuilder& BufferBuilder::withUsage(VkBufferUsageFlags usage) noexcept {
  _metadata.usage = usage;
  return *this;
}

BufferBuilder& BufferBuilder::withSize(VkDeviceSize size) noexcept {
  _metadata.size = size;
  return *this;
}

BufferBuilder& BufferBuilder::withFlags(VkBufferCreateFlags flags) noexcept {
  _metadata.flags = flags;
  return *this;
}

BufferBuilder& BufferBuilder::withQueueFamilyIndices(
    std::span<const uint32_t> queueFamilyIndices) noexcept {
  _metadata.sharingMode = VK_SHARING_MODE_CONCURRENT;
  _metadata.queueFamilyIndices.assign(queueFamilyIndices.cbegin(), queueFamilyIndices.cend());
  return *this;
}

BufferMetadata BufferBuilder::getMetadata() const noexcept {
  return _metadata;
}
