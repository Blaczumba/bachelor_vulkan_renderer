#pragma once

#include <array>
#include <cstring>
#include <format>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/buffer_manip.h"
#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"

class Buffer {
public:
  Buffer() noexcept = default;

  Buffer(Buffer&& Buffer) noexcept;

  Buffer& operator=(Buffer&& Buffer) noexcept;

  ~Buffer();

  static Buffer createVertexInputBuffer(
      const LogicalDevice& logicalDevice, uint32_t size, VkBufferUsageFlags usage = {});

  static Buffer createStagingBuffer(
      const LogicalDevice& logicalDevice, uint32_t size, VkBufferUsageFlags usage = {});

  static Buffer createUniformBuffer(const LogicalDevice& logicalDevice, uint32_t size);

  std::span<const std::byte> getMappedMemory() const noexcept;

  std::span<std::byte> getMappedMemory() noexcept;

  void copyBuffer(const VkCommandBuffer commandBuffer, const Buffer& srcBuffer,
                  std::optional<VkDeviceSize> size = std::nullopt, VkDeviceSize srcOffset = 0,
                  VkDeviceSize dstOffset = 0);

  VkBufferUsageFlags getUsage() const noexcept;

  uint32_t getSize() const noexcept;

  const VkBuffer& getVkBuffer() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  Buffer(const LogicalDevice& logicalDevice, const Allocation allocation,
         const VkBuffer vertexBuffer, VkBufferUsageFlags usage, uint32_t size,
         void* mappedData = nullptr) noexcept;

  void destroy();

  VkBuffer _buffer = VK_NULL_HANDLE;
  Allocation _allocation;
  VkDeviceSize _size;
  VkBufferUsageFlags _usage;
  void* _mappedMemory = nullptr;

  const LogicalDevice* _logicalDevice;
};
