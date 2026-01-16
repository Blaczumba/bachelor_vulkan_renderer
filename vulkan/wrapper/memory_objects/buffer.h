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

  static Buffer createVertexBuffer(const LogicalDevice& logicalDevice, uint32_t size);

  static Buffer createIndexBuffer(const LogicalDevice& logicalDevice, uint32_t size);

  static Buffer createStagingBuffer(
      const LogicalDevice& logicalDevice, uint32_t size, VkBufferUsageFlags additionalUsage = {});

  static Buffer createUniformBuffer(const LogicalDevice& logicalDevice, uint32_t size);

  void copyBuffer(const VkCommandBuffer commandBuffer, const Buffer& srcBuffer,
                  std::optional<VkDeviceSize> size = std::nullopt, VkDeviceSize srcOffset = 0,
                  VkDeviceSize dstOffset = 0);

  void copyDataInterleaving(
      std::span<const glm::vec3> positions, std::span<const glm::vec2> texCoords);

  void copyDataInterleaving(
      std::span<const glm::vec3> positions, std::span<const glm::vec2> texCoords,
      std::span<const glm::vec3> normals);

  void copyDataInterleaving(std::span<const AttributeDescription> attributes);

  void copyAndShrinkData(std::span<const std::byte> data, size_t dstIndexSize, size_t srcIndexSize,
                         VkDeviceSize offset = 0);

  template <typename T>
  void copyData(std::span<const T> data, VkDeviceSize offset = 0);

  template <typename T>
  void copyData(const T& data, VkDeviceSize offset = 0);

  VkBufferUsageFlags getUsage() const;

  uint32_t getSize() const;

  void* getMappedMemory() const;

  const VkBuffer& getVkBuffer() const;

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

template <typename T>
void Buffer::copyData(std::span<const T> data, VkDeviceSize offset) {
  if (!_mappedMemory) [[unlikely]] {
    throw EngineException("Cannot copy raw data to unmapped memory.");
  }
  const uint32_t size = data.size() * sizeof(T);
  if (offset + size > _size) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        offset, size, _size));
  }
  std::memcpy(static_cast<uint8_t*>(_mappedMemory) + offset, data.data(), size);
}

template <typename T>
void Buffer::copyData(const T& data, VkDeviceSize offset) {
  if (!_mappedMemory) [[unlikely]] {
    throw EngineException("Cannot copy raw data to unmapped memory.");
  }
  if (offset + sizeof(T) > _size) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        offset, sizeof(T), _size));
  }
  std::memcpy(static_cast<uint8_t*>(_mappedMemory) + offset, &data, sizeof(T));
}
