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

Buffer::Buffer(
    const LogicalDevice& logicalDevice, const Allocation allocation, const VkBuffer buffer) noexcept
  : _logicalDevice(&logicalDevice), _allocation(allocation), _buffer(buffer) {}

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

}  // namespace

BufferWithMetadata BufferBuilder::createVertexInputBuffer(
    const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      std::visit(VertexInputBufferAllocator{_createInfo}, logicalDevice.getMemoryAllocator());
  return std::make_tuple(Buffer(logicalDevice, bufferResources.allocation, bufferResources.buffer),
      BufferMetadata{
        _createInfo.usage, _createInfo.size, reinterpret_cast<std::byte*>(bufferResources.mappedMemory),
        _createInfo.flags, _createInfo.sharingMode});
}

BufferWithMetadata BufferBuilder::createStagingBuffer(
    const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      std::visit(StagingBufferAllocator{_createInfo}, logicalDevice.getMemoryAllocator());
  return std::make_tuple(Buffer(logicalDevice, bufferResources.allocation, bufferResources.buffer),
      BufferMetadata{
        _createInfo.usage, _createInfo.size, reinterpret_cast<std::byte*>(bufferResources.mappedMemory),
        _createInfo.flags, _createInfo.sharingMode});
}

BufferWithMetadata BufferBuilder::createUniformBuffer(
    const LogicalDevice& logicalDevice) {
  const BufferResources bufferResources =
      std::visit(UniformBufferAllocator{_createInfo}, logicalDevice.getMemoryAllocator());
  return std::make_tuple(Buffer(logicalDevice, bufferResources.allocation, bufferResources.buffer),
      BufferMetadata{
        _createInfo.usage, _createInfo.size, reinterpret_cast<std::byte*>(bufferResources.mappedMemory),
        _createInfo.flags, _createInfo.sharingMode});
}

const VkBuffer& Buffer::getVkBuffer() const noexcept {
  return _buffer;
}

const LogicalDevice& Buffer::getLogicalDevice() const {
  return *_logicalDevice;
}

BufferBuilder& BufferBuilder::withUsage(VkBufferUsageFlags usage) noexcept {
  _createInfo.usage = usage;
  return *this;
}

BufferBuilder& BufferBuilder::withSize(VkDeviceSize size) noexcept {
  _createInfo.size = size;
  return *this;
}

BufferBuilder& BufferBuilder::withFlags(VkBufferCreateFlags flags) noexcept {
  _createInfo.flags = flags;
  return *this;
}

BufferBuilder& BufferBuilder::withQueueFamilyIndices(
    std::span<const uint32_t> queueFamilyIndices) noexcept {
  _createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
  _createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
  _createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  return *this;
}
