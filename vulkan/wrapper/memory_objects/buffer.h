#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

struct BufferMetadata {
  VkBufferUsageFlags usage;
  VkDeviceSize size;
  std::byte* mappedMemory;
  VkBufferCreateFlags flags;
  VkSharingMode sharingMode;
  // Other std::optional fields representing pNext metadata.
  std::span<const std::byte> getMappedMemoryAsSpan() const noexcept;

  std::span<std::byte> getMappedMemoryAsSpan() noexcept;
};

class Buffer {
  Buffer(const LogicalDevice& logicalDevice, VkBuffer buffer, Allocation allocation) noexcept;

public:
  Buffer() noexcept = default;

  Buffer(Buffer&& buffer) noexcept;

  Buffer& operator=(Buffer&& buffer) noexcept;

  ~Buffer();

  const VkBuffer& getVkBuffer() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  void destroy();

  VkBuffer _buffer = VK_NULL_HANDLE;
  Allocation _allocation;

  const LogicalDevice* _logicalDevice;

  friend class BufferBuilder;
};

struct BufferWithMetadata {
  Buffer buffer;
  BufferMetadata metadata;
};

class BufferBuilder {
public:
  BufferBuilder& withUsage(VkBufferUsageFlags usage) noexcept;

  BufferBuilder& withSize(VkDeviceSize size) noexcept;

  BufferBuilder& withFlags(VkBufferCreateFlags flags) noexcept;

  BufferBuilder& withQueueFamilyIndices(std::span<const uint32_t> queueFamilyIndices) noexcept;

  BufferWithMetadata createVertexInputBuffer(const LogicalDevice& logicalDevice);

  BufferWithMetadata createStagingBuffer(const LogicalDevice& logicalDevice);

  BufferWithMetadata createUniformBuffer(const LogicalDevice& logicalDevice);

private:
  VkBufferCreateInfo _createInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
};
