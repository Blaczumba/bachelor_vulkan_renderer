#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <span>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

class CommandBuffer;

class CommandPool : public std::enable_shared_from_this<const CommandPool> {
  CommandPool(const LogicalDevice& logicalDevice, VkCommandPool commandPool) noexcept;

public:
  static std::unique_ptr<CommandPool> create(
      const LogicalDevice& logicalDevice, VkCommandPoolCreateFlags flags = 0);

  ~CommandPool();

  CommandBuffer createCommandBuffer(VkCommandBufferLevel level) const;

  std::vector<CommandBuffer> createCommandBuffers(VkCommandBufferLevel level, uint32_t count) const;

  template <size_t COUNT>
  std::array<CommandBuffer, COUNT> createCommandBuffers(VkCommandBufferLevel level) const;

  void reset() const;

  VkCommandPool getVkCommandPool() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  VkCommandPool _commandPool;
  const LogicalDevice& _logicalDevice;
};

class CommandBuffer {
  CommandBuffer(const std::shared_ptr<const CommandPool>& commandPool,
                VkCommandBuffer commandBuffer, VkCommandBufferLevel level) noexcept;

public:
  CommandBuffer() noexcept = default;

  static CommandBuffer create(
      const std::shared_ptr<const CommandPool>& commandPool, VkCommandBufferLevel level);

  template <size_t COUNT>
  static std::array<CommandBuffer, COUNT> create(
      const std::shared_ptr<const CommandPool>& commandPool, VkCommandBufferLevel level);

  static std::vector<CommandBuffer> create(const std::shared_ptr<const CommandPool>& commandPool,
                                           VkCommandBufferLevel level, uint32_t count);

  CommandBuffer(CommandBuffer&&) noexcept;

  CommandBuffer& operator=(CommandBuffer&&) noexcept;

  ~CommandBuffer();

  void beginRenderPass(const Framebuffer& framebuffer, VkExtent2D framebufferExtent,
                       std::span<const VkClearValue> clearValues) const;

  void setVieport(
      std::span<const VkViewport> viewports, uint32_t firstVieport = 0) const noexcept;

  void setScissor(
      std::span<const VkRect2D> scissors, uint32_t firstScissor = 0) const noexcept;

  void endRenderPass() const;

  void beginAsPrimary(uint32_t subpassIndex = 0) const;

  void beginAsSecondary(
      const Framebuffer& framebuffer,
      const VkCommandBufferInheritanceViewportScissorInfoNV* scissorViewportInheritance = nullptr,
      uint32_t subpassIndex = 0) const;

  VkResult end() const;

  void executeSecondaryCommandBuffers(std::initializer_list<VkCommandBuffer> commandBuffers) const;

  void submit(QueueType type, const VkSemaphore waitSemaphore, const VkSemaphore signalSemaphore,
              const VkFence waitFence) const;

  void resetCommandBuffer() const;

  VkCommandBuffer getVkCommandBuffer() const noexcept;

private:
  VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
  std::shared_ptr<const CommandPool> _commandPool;

  VkCommandBufferLevel _level;
};

template <size_t COUNT>
std::array<CommandBuffer, COUNT> CommandPool::createCommandBuffers(
    VkCommandBufferLevel level) const {
  return CommandBuffer::create<COUNT>(shared_from_this(), level);
}

namespace {

VkResult createCommandBuffers(
    VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level,
    std::span<VkCommandBuffer> outCommandBuffers) {
  const VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPool,
    .level = level,
    .commandBufferCount = static_cast<uint32_t>(outCommandBuffers.size()),
  };
  return vkAllocateCommandBuffers(device, &allocInfo, outCommandBuffers.data());
}

}  // namespace

template <size_t COUNT>
std::array<CommandBuffer, COUNT> CommandBuffer::create(
    const std::shared_ptr<const CommandPool>& commandPool, VkCommandBufferLevel level) {
  VkCommandBuffer vkCommandBuffers[COUNT];
  CHECK_VKCMD(createCommandBuffers(commandPool->getLogicalDevice().getVkDevice(),
                                   commandPool->getVkCommandPool(), level, vkCommandBuffers),
              "Failed to create VkCommandBuffer.");

  std::array<CommandBuffer, COUNT> commandBuffers;
  std::transform(std::cbegin(vkCommandBuffers), std::cend(vkCommandBuffers), commandBuffers.begin(),
                 [&commandPool, level](VkCommandBuffer commandBuffer) {
                   return CommandBuffer(commandPool, commandBuffer, level);
                 });
  return commandBuffers;
}

class SingleTimeCommandBuffer {
  VkCommandBuffer _commandBuffer;
  VkFence _fence;
  const QueueType _queueType;

  const CommandPool& _commandPool;

public:
  SingleTimeCommandBuffer(
      const CommandPool& commandPool, QueueType queueType = QueueType::GRAPHICS);

  ~SingleTimeCommandBuffer();

  VkCommandBuffer getCommandBuffer() const noexcept;
};
