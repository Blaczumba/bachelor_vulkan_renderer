#pragma once

#include <variant>

#include "common/status/status.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/physical_device/physical_device.h"
#include "vulkan/wrapper/logical_device/resource_destroyer.h"

enum class QueueType : uint8_t {
  GRAPHICS = 0,
  PRESENT,
  COMPUTE,
  TRANSFER
};

class LogicalDevice {
  VkDevice _device = VK_NULL_HANDLE;

  const PhysicalDevice* _physicalDevice = nullptr;
  mutable MemoryAllocator _memoryAllocator;
  std::unique_ptr<ResourceDestroyer> _resourceDestroyer;

  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkQueue _presentQueue = VK_NULL_HANDLE;
  VkQueue _computeQueue = VK_NULL_HANDLE;
  VkQueue _transferQueue = VK_NULL_HANDLE;

  LogicalDevice(VkDevice logicalDevice, const PhysicalDevice& physicalDevice,
                std::unique_ptr<ResourceDestroyer>&& resourceDestroyer);

public:
  LogicalDevice() = default;

  static ErrorOr<LogicalDevice> create(
      const PhysicalDevice& physicalDevice,
      std::unique_ptr<ResourceDestroyer>&& resourceDestroyer = std::
          make_unique<ImmediateResourceDestroyer>());

  static ErrorOr<LogicalDevice> wrap(VkDevice device, const PhysicalDevice& physicalDevice,
                                     std::unique_ptr<ResourceDestroyer>&& resourceDestroyer = std::
                                         make_unique<ImmediateResourceDestroyer>());

  LogicalDevice(LogicalDevice&& logicalDevice) noexcept;

  LogicalDevice& operator=(LogicalDevice&& logicalDevice) noexcept;

  ~LogicalDevice();

  void destroyResource(ResourceDestroyerJob&& destroyResource) const;

  ErrorOr<VkSampler> createSampler(const SamplerParameters& params) const;

  ErrorOr<VkImageView> createImageView(
      VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect,
      uint32_t baseMipLevel, uint32_t mipLevels, uint32_t baseArrayLayer,
      uint32_t layerCount) const;

  VkDevice getVkDevice() const;

  const PhysicalDevice& getPhysicalDevice() const;

  MemoryAllocator& getMemoryAllocator() const;

  VkQueue getVkQueue(QueueType queueType) const;

  VkQueue getGraphicsVkQueue() const;

  VkQueue getPresentVkQueue() const;

  VkQueue getComputeVkQueue() const;

  VkQueue getTransferVkQueue() const;
};
