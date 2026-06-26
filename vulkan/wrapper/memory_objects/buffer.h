#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class Buffer {
  Buffer(const LogicalDevice& logicalDevice, VkBuffer buffer, Allocation allocation) noexcept;

public:
  Buffer() noexcept = default;

  Buffer(Buffer&& buffer) noexcept;

  Buffer& operator=(Buffer&& buffer) noexcept;

  ~Buffer();

  const VkBuffer& getVkBuffer() const noexcept;

  const LogicalDevice& getLogicalDevice() const noexcept;

private:
  void destroy();

  VkBuffer _buffer = VK_NULL_HANDLE;
  Allocation _allocation;

  const LogicalDevice* _logicalDevice;

  friend class BufferBuilder;
};

struct BufferMetadata {
  VkBufferUsageFlags usage;
  VkDeviceSize size;
  std::byte* mappedMemory;
  VkBufferCreateFlags flags;
  VkSharingMode sharingMode;
  std::vector<uint32_t> queueFamilyIndices;
  // Other std::optional fields representing pNext metadata.
  std::span<const std::byte> getMappedMemoryAsSpan() const noexcept;

  std::span<std::byte> getMappedMemoryAsSpan() noexcept;
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

  BufferMetadata getMetadata() const noexcept;

  Buffer createVertexInputBuffer(const LogicalDevice& logicalDevice);

  Buffer createStagingBuffer(const LogicalDevice& logicalDevice);

  Buffer createUniformBuffer(const LogicalDevice& logicalDevice);

private:
  BufferMetadata _metadata = {
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
};
