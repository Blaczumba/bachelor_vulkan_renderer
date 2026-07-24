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
BufferResources createBuffer(
    const LogicalDevice& logicalDevice, const VkBufferCreateInfo& createInfo) {
  return std::visit(Allocator{createInfo}, logicalDevice.getMemoryAllocator());
}

}  // namespace

const VkBuffer& Buffer::getVkBuffer() const noexcept {
  return _buffer;
}

const LogicalDevice& Buffer::getLogicalDevice() const noexcept {
  return *_logicalDevice;
}

BufferBuilder& BufferBuilder::withUsage(VkBufferUsageFlags usage) noexcept {
  _usage = usage;
  return *this;
}

BufferBuilder& BufferBuilder::withSize(VkDeviceSize size) noexcept {
  _size = size;
  return *this;
}

BufferBuilder& BufferBuilder::withFlags(VkBufferCreateFlags flags) noexcept {
  _flags = flags;
  return *this;
}

BufferBuilder& BufferBuilder::withQueueFamilyIndices(
    std::span<const uint32_t> queueFamilyIndices) noexcept {
  _sharingMode = VK_SHARING_MODE_CONCURRENT;
  _queueFamilyIndices.assign(std::cbegin(queueFamilyIndices), std::cend(queueFamilyIndices));
  return *this;
}

BufferMetadata BufferBuilder::getMetadata() const noexcept {
  return BufferMetadata{
    .usage = _usage,
    .size = _size,
    .mappedMemory = _mappedMemory,
    .flags = _flags,
    .sharingMode = _sharingMode,
    .queueFamilyIndices = _queueFamilyIndices,
  };
}

VkBufferCreateInfo BufferBuilder::getCreateInfo() const noexcept {
  return VkBufferCreateInfo{
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = _flags,
    .size = _size,
    .usage = _usage,
    .sharingMode = _sharingMode,
    .queueFamilyIndexCount = static_cast<uint32_t>(_queueFamilyIndices.size()),
    .pQueueFamilyIndices = _queueFamilyIndices.empty() ? nullptr : _queueFamilyIndices.data(),
  };
}

Buffer BufferBuilder::createVertexInputBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<VertexInputBufferAllocator>(logicalDevice, getCreateInfo());
  _mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}

Buffer BufferBuilder::createStagingBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<StagingBufferAllocator>(logicalDevice, getCreateInfo());
  _mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}

Buffer BufferBuilder::createUniformBuffer(const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      createBuffer<UniformBufferAllocator>(logicalDevice, getCreateInfo());
  _mappedMemory = reinterpret_cast<std::byte*>(bufferResources.mappedMemory);
  return Buffer(logicalDevice, bufferResources.buffer, bufferResources.allocation);
}
