#include "buffer.h"

#include <format>
#include <glm/glm.hpp>
#include <iterator>
#include <numeric>

#include "vulkan/wrapper/memory_objects/buffers.h"

Buffer::Buffer(
    const LogicalDevice& logicalDevice, const Allocation allocation, const VkBuffer buffer,
    VkBufferUsageFlags usage, uint32_t size, void* mappedData) noexcept
  : _logicalDevice(&logicalDevice), _allocation(allocation), _buffer(buffer), _usage(usage),
    _size(size), _mappedMemory(mappedData) {}

Buffer::Buffer(Buffer&& buffer) noexcept
  : _buffer(std::exchange(buffer._buffer, VK_NULL_HANDLE)), _allocation(buffer._allocation),
    _logicalDevice(std::exchange(buffer._logicalDevice, nullptr)), _usage(buffer._usage),
    _size(buffer._size), _mappedMemory(std::exchange(buffer._mappedMemory, nullptr)) {}

Buffer& Buffer::operator=(Buffer&& buffer) noexcept {
  if (this == &buffer) {
    return *this;
  }

  destroy();

  _buffer = std::exchange(buffer._buffer, VK_NULL_HANDLE);
  _allocation = buffer._allocation;
  _size = buffer._size;
  _usage = buffer._usage;
  _mappedMemory = std::exchange(buffer._mappedMemory, nullptr);
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
  if (_buffer != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource(
        [buffer = _buffer, allocation = _allocation](DestroyerContext context) {
          std::visit(BufferDeallocator{buffer}, *context.memoryAllocator, allocation);
        });
  }
}

Buffer::~Buffer() {
  destroy();
  _buffer = VK_NULL_HANDLE;
}

namespace {

struct BufferData {
  VkBuffer buffer;
  Allocation allocation;
  VkBufferUsageFlags usage;
  void* mappedMemory;
};

struct VertexInputBufferAllocator {
  const size_t size;
  VkBufferUsageFlags usage;

  BufferData operator()(VmaWrapper& allocator) {
    const VmaWrapper::Buffer buffer =
        allocator.createVkBuffer(size, usage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    return BufferData{buffer.buffer, buffer.allocation, usage};
  }

  BufferData operator()(auto&&) {
    return {};
  }
};

struct StagingBufferAllocator {
  const size_t size;
  VkBufferUsageFlags usage = {};

  BufferData operator()(VmaWrapper& wrapper) {
    const VmaWrapper::Buffer buffer = wrapper.createVkBuffer(
        size, usage, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    return BufferData{buffer.buffer, buffer.allocation, usage, buffer.mappedData};
  }

  BufferData operator()(auto&&) {
    return {};
  }
};

struct UniformBufferAllocator {
  const size_t size;

  BufferData operator()(VmaWrapper& allocator) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    const VmaWrapper::Buffer buffer = allocator.createVkBuffer(
        size, usage, VMA_MEMORY_USAGE_CPU_ONLY,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    return BufferData{buffer.buffer, buffer.allocation, usage, buffer.mappedData};
  }

  BufferData operator()(auto&&) {
    return {};
  }
};

}  // namespace

Buffer Buffer::createVertexInputBuffer(const LogicalDevice& logicalDevice, uint32_t size, VkBufferUsageFlags usage) {
  const BufferData bufferData =
      std::visit(VertexInputBufferAllocator{size, usage}, logicalDevice.getMemoryAllocator());
  return Buffer(logicalDevice, bufferData.allocation, bufferData.buffer, bufferData.usage, size,
                bufferData.mappedMemory);
}

Buffer Buffer::createStagingBuffer(
    const LogicalDevice& logicalDevice, uint32_t size, VkBufferUsageFlags usage) {
  const BufferData bufferData =
      std::visit(StagingBufferAllocator{size, usage}, logicalDevice.getMemoryAllocator());
  return Buffer(logicalDevice, bufferData.allocation, bufferData.buffer, bufferData.usage, size,
                bufferData.mappedMemory);
}

Buffer Buffer::createUniformBuffer(const LogicalDevice& logicalDevice, uint32_t size) {
  const BufferData bufferData =
      std::visit(UniformBufferAllocator{size}, logicalDevice.getMemoryAllocator());
  return Buffer(logicalDevice, bufferData.allocation, bufferData.buffer, bufferData.usage, size,
                bufferData.mappedMemory);
}

void Buffer::copyBuffer(
    const VkCommandBuffer commandBuffer, const Buffer& srcBuffer,
    std::optional<VkDeviceSize> srcSize, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
  if ((_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) [[unlikely]] {
    throw EngineException(
        "When copying one buffer to other the destination one must have "
        "VK_BUFFER_USAGE_TRANSFER_DST_BIT specified.");
  }

  if ((srcBuffer._usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) [[unlikely]] {
    throw EngineException(
        "When copying one buffer to other the source one must have "
        "VK_BUFFER_USAGE_TRANSFER_SRC_BIT specified.");
  }

  const VkDeviceSize size = srcSize.value_or(srcBuffer._size);
  if (srcOffset + size > srcBuffer._size) [[unlikely]] {
    // TODO:
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        srcOffset, size, srcBuffer._size));
  }

  if (dstOffset + size > _size) [[unlikely]] {
    // TODO:
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        srcOffset, size, srcBuffer._size));
  }

  copyBufferToBuffer(commandBuffer, srcBuffer._buffer, _buffer, srcOffset, dstOffset, size);
}

void Buffer::copyAndShrinkData(std::span<const std::byte> data, size_t dstIndexSize,
                               size_t srcIndexSize, VkDeviceSize offset) {
  if (!_mappedMemory) [[unlikely]] {
    throw EngineException("Cannot copy raw data to unmapped memory.");
  }

  if (_size < dstIndexSize * data.size() / srcIndexSize + offset) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        offset, dstIndexSize * data.size() / srcIndexSize, _size));
  }

  copyAndShrinkIndices(static_cast<uint8_t*>(_mappedMemory) + offset, dstIndexSize, data.data(),
                       srcIndexSize, data.size() / srcIndexSize);
}

void Buffer::copyDataInterleaving(std::span<const AttributeDescription> attributes) {
  if (!_mappedMemory) [[unlikely]] {
    throw EngineException("Cannot copy raw data to unmapped memory.");
  }

  if (std::any_of(std::cbegin(attributes), std::cend(attributes),
                  [first = attributes[0].count](const AttributeDescription& attribute) {
                    return attribute.count != first;
                  })) {
    throw EngineException(
        "When copying buffers in an interleaving manner the buffers must have equal number of "
        "elements.");
  }

  std::vector<uint8_t*> offsetMemory;
  offsetMemory.reserve(attributes.size());
  offsetMemory.push_back(static_cast<uint8_t*>(_mappedMemory));
  size_t stride = 0;
  std::transform(
      std::cbegin(attributes), std::prev(std::cend(attributes)), std::back_inserter(offsetMemory),
      [this, &stride](const AttributeDescription& attribute) {
        stride += attribute.size;
        return static_cast<uint8_t*>(_mappedMemory) + stride;
      });
  stride += attributes.back().size;

  for (size_t j = 0, running_stride = 0; j < attributes[0].count; j++, running_stride += stride) {
    for (size_t i = 0; i < attributes.size(); i++) {
      const AttributeDescription& attribute = attributes[i];
      std::memcpy(offsetMemory[i] + running_stride,
                  static_cast<uint8_t*>(attribute.data) + j * attribute.size, attribute.size);
    }
  }
}

VkBufferUsageFlags Buffer::getUsage() const noexcept {
  return _usage;
}

uint32_t Buffer::getSize() const noexcept {
  return _size;
}

void* Buffer::getMappedMemory() const noexcept {
  return _mappedMemory;
}

const VkBuffer& Buffer::getVkBuffer() const noexcept {
  return _buffer;
}

const LogicalDevice& Buffer::getLogicalDevice() const {
  return *_logicalDevice;
}
