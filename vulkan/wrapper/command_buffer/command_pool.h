#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class CommandBuffer;

class CommandPool : public std::enable_shared_from_this<const CommandPool> {
  CommandPool(const LogicalDevice& logicalDevice, VkCommandPool commandPool) noexcept;

public:
  static std::unique_ptr<CommandPool> create(
      const LogicalDevice& logicalDevice, VkCommandPoolCreateFlags flags = 0);

  ~CommandPool();

  CommandBuffer createCommandBuffer(VkCommandBufferLevel level) const;

  std::vector<CommandBuffer> createCommandBuffers(VkCommandBufferLevel level, uint32_t count) const;

  // NOTE: The definition of this template lives in command_buffer.h (it needs the
  // full CommandBuffer type). Include command_buffer.h in any TU that calls it.
  template <size_t COUNT>
  std::array<CommandBuffer, COUNT> createCommandBuffers(VkCommandBufferLevel level) const;

  void reset() const;

  VkCommandPool getVkCommandPool() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  VkCommandPool _commandPool;
  const LogicalDevice& _logicalDevice;
};
