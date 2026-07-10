#include "command_buffer.h"

#include <cstdint>
#include <iterator>
#include <memory>
#include <span>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/command_buffer/command_pool.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/logical_device/resource_destroyer.h"
#include "vulkan/wrapper/util/check.h"

namespace {

template <typename T>
void chainExtendedField(void** next, T& feature) {
  feature.pNext = *next;
  *next = (void*)&feature;
}

}  // namespace

CommandBuffer::CommandBuffer(const std::shared_ptr<const CommandPool>& commandPool,
                             VkCommandBuffer commandBuffer, VkCommandBufferLevel level) noexcept
  : _commandPool(commandPool), _commandBuffer(commandBuffer), _level(level) {}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
  : _commandPool(std::move(other._commandPool)),
    _commandBuffer(std::exchange(other._commandBuffer, VK_NULL_HANDLE)), _level(other._level) {}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  _commandPool = std::move(other._commandPool);
  _commandBuffer = std::exchange(other._commandBuffer, VK_NULL_HANDLE);
  _level = other._level;
  return *this;
}

CommandBuffer::~CommandBuffer() {
  if (_commandBuffer != VK_NULL_HANDLE) {
    _commandPool->getLogicalDevice().destroyResource(
        [pool = _commandPool->getVkCommandPool(),
         buffer = _commandBuffer](DestroyerContext context) {
          vkFreeCommandBuffers(context.device, pool, 1, &buffer);
        });
  }
}

CommandBuffer CommandBuffer::create(
    const std::shared_ptr<const CommandPool>& commandPool, VkCommandBufferLevel level) {
  VkCommandBuffer commandBuffer;
  CHECK_VKCMD(
      internal::allocateCommandBuffers(commandPool->getLogicalDevice().getVkDevice(),
                                       commandPool->getVkCommandPool(), level, {&commandBuffer, 1}),
      "Failed to create VkCommandBuffer.");
  return CommandBuffer(commandPool, commandBuffer, level);
}

std::vector<CommandBuffer> CommandBuffer::create(
    const std::shared_ptr<const CommandPool>& commandPool, VkCommandBufferLevel level,
    uint32_t count) {
  lib::Buffer<VkCommandBuffer> vkCommandBuffers(count);
  CHECK_VKCMD(
      internal::allocateCommandBuffers(commandPool->getLogicalDevice().getVkDevice(),
                                       commandPool->getVkCommandPool(), level, vkCommandBuffers),
      "Failed to create VkCommandBuffer.");
  std::vector<CommandBuffer> commandBuffers;
  commandBuffers.reserve(vkCommandBuffers.size());
  std::transform(
      std::cbegin(vkCommandBuffers), std::cend(vkCommandBuffers),
      std::back_inserter(commandBuffers), [&commandPool, level](VkCommandBuffer commandBuffer) {
        return CommandBuffer(commandPool, commandBuffer, level);
      });
  return commandBuffers;
}

void CommandBuffer::beginRenderPass(const Framebuffer& framebuffer, VkExtent2D framebufferExtent,
                                    std::span<const VkClearValue> clearValues) const {
  if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) [[unlikely]] {
    throw EngineException(
        "Cannot begin renderpass without VK_COMMAND_BUFFER_LEVEL_PRIMARY specified.");
  }

  const Renderpass& renderpass = framebuffer.getRenderpass();
  const VkRenderPassBeginInfo renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = renderpass.getVkRenderPass(),
    .framebuffer = framebuffer.getVkFramebuffer(),
    .renderArea = {.offset = {0, 0}, .extent = framebufferExtent},
    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
    .pClearValues = clearValues.data()
  };
  vkCmdBeginRenderPass(
      _commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void CommandBuffer::endRenderPass() const {
  vkCmdEndRenderPass(_commandBuffer);
}

void CommandBuffer::setVieport(
    std::span<const VkViewport> viewports, uint32_t firstVieport) const noexcept {
  vkCmdSetViewport(
      _commandBuffer, firstVieport, static_cast<uint32_t>(viewports.size()), viewports.data());
}

void CommandBuffer::setScissor(
    std::span<const VkRect2D> scissors, uint32_t firstScissor) const noexcept {
  vkCmdSetScissor(
      _commandBuffer, firstScissor, static_cast<uint32_t>(scissors.size()), scissors.data());
}

void CommandBuffer::bindPipeline(
    VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const noexcept {
  vkCmdBindPipeline(_commandBuffer, pipelineBindPoint, pipeline);
}

void CommandBuffer::bindVertexBuffers(
    std::span<const VkBuffer> buffers, std::span<const VkDeviceSize> offsets,
    uint32_t firstBinding) const noexcept {
  vkCmdBindVertexBuffers(_commandBuffer, firstBinding, static_cast<uint32_t>(buffers.size()),
                         buffers.data(), offsets.data());
}

void CommandBuffer::bindIndexBuffer(
    VkBuffer buffer, VkIndexType indexType, VkDeviceSize offset) const noexcept {
  vkCmdBindIndexBuffer(_commandBuffer, buffer, offset, indexType);
}

void CommandBuffer::bindDescriptorSets(
    VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
    std::span<const VkDescriptorSet> descriptorSets, uint32_t firstSet,
    std::span<const uint32_t> dynamicOffsets) const noexcept {
  vkCmdBindDescriptorSets(_commandBuffer, pipelineBindPoint, layout, firstSet,
                          static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
                          static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
}

void CommandBuffer::pushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                  std::span<const std::byte> data, uint32_t offset) const noexcept {
  vkCmdPushConstants(
      _commandBuffer, layout, stageFlags, offset, static_cast<uint32_t>(data.size()), data.data());
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                int32_t vertexOffset, uint32_t firstInstance) const noexcept {
  vkCmdDrawIndexed(
      _commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::executeSecondaryCommandBuffers(
    std::span<const VkCommandBuffer> commandBuffers) const {
  if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) [[unlikely]] {
    throw EngineException(
        "Secondary command buffers can only be executed from the primary command buffer.");
  }

  vkCmdExecuteCommands(
      _commandBuffer, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void CommandBuffer::submit(QueueType type, const VkSemaphore waitSemaphore,
                           const VkSemaphore signalSemaphore, const VkFence waitFence) const {
  if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) [[unlikely]] {
    throw EngineException("Secondary command buffers cannot be submitted directly to the queue.");
  }

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {waitSemaphore};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  if (waitSemaphore != VK_NULL_HANDLE) {
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
  }

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_commandBuffer;

  VkSemaphore signalSemaphores[] = {signalSemaphore};
  if (signalSemaphore != VK_NULL_HANDLE) {
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
  }

  const LogicalDevice& logicalDevice = _commandPool->getLogicalDevice();
  if (waitFence != VK_NULL_HANDLE) {
    CHECK_VKCMD(vkResetFences(logicalDevice.getVkDevice(), 1, &waitFence),
                "Failed to vkResetFences in CommandBuffer::submit.");
  }

  CHECK_VKCMD(vkQueueSubmit(logicalDevice.getVkQueue(type), 1, &submitInfo, waitFence),
              "Failed to vkQueueSubmit in CommandBuffer::submit.");
}

void CommandBuffer::beginAsPrimary() const {
  if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) [[unlikely]] {
    throw EngineException(
        "Cannot begin command buffer as primary without VK_COMMAND_BUFFER_LEVEL_PRIMARY "
        "specified.");
  }

  const VkCommandBufferBeginInfo beginInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                              .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
  CHECK_VKCMD(vkBeginCommandBuffer(_commandBuffer, &beginInfo),
              "Failed to vkBeginCommandBuffer for primary command buffer.");
}

CommandBuffer::BeginInfoBuilder& CommandBuffer::BeginInfoBuilder::
    withViewportScissorInheritenceInfo(std::span<const VkViewport> viewports) {
  if (_viewportScissorInheritanceInfo.has_value()) [[unlikely]] {
    return *this;
  }

  _viewportScissorInheritanceInfo = VkCommandBufferInheritanceViewportScissorInfoNV{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV,
    .viewportScissor2D = VK_TRUE,
    .viewportDepthCount = static_cast<uint32_t>(viewports.size()),
    .pViewportDepths = viewports.data()};

  chainExtendedField(&_inheritenceInfoPNext, *_viewportScissorInheritanceInfo);
  return *this;
}

CommandBuffer::BeginInfoBuilder& CommandBuffer::BeginInfoBuilder::withInheritenceInfo(
    VkRenderPass renderpass, VkFramebuffer framebuffer, uint32_t subpass,
    std::optional<VkQueryControlFlags> queryControlFlags,
    VkQueryPipelineStatisticFlags pipelineStatistics) {
  if (_inheritanceInfo.has_value()) [[unlikely]] {
    return *this;
  }

  _inheritanceInfo = VkCommandBufferInheritanceInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
    .renderPass = renderpass,
    .subpass = subpass,
    .framebuffer = framebuffer,
    .occlusionQueryEnable = queryControlFlags.has_value() ? VK_TRUE : VK_FALSE,
    .queryFlags = queryControlFlags.value_or(0),
    .pipelineStatistics = pipelineStatistics};

  return *this;
}

void CommandBuffer::BeginInfoBuilder::beginCommandBuffer(
    const CommandBuffer& commandBuffer, VkCommandBufferUsageFlags usageFlags) {
  if (commandBuffer._level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
    if (!_inheritanceInfo.has_value()) {
      throw EngineException("Inheritance info must be specified for secondary command buffers!");
    }
    _inheritanceInfo->pNext = _inheritenceInfoPNext;
  }

  const VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = _pNext,
    .flags = usageFlags,
    .pInheritanceInfo = _inheritanceInfo.has_value() ? &_inheritanceInfo.value() : nullptr};

  CHECK_VKCMD(vkBeginCommandBuffer(commandBuffer.getVkCommandBuffer(), &beginInfo),
              "Failed to vkBeginCommandBuffer for secondary command buffer.");
}

VkResult CommandBuffer::end() const {
  return vkEndCommandBuffer(_commandBuffer);
}

void CommandBuffer::resetCommandBuffer() const {
  vkResetCommandBuffer(_commandBuffer, 0);
}

VkCommandBuffer CommandBuffer::getVkCommandBuffer() const noexcept {
  return _commandBuffer;
}

SingleTimeCommandBuffer::SingleTimeCommandBuffer(
    const CommandPool& commandPool, QueueType queueType)
  : _commandPool(commandPool), _queueType(queueType) {
  const VkDevice device = _commandPool.getLogicalDevice().getVkDevice();
  const VkFenceCreateInfo fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

  vkCreateFence(device, &fenceInfo, nullptr, &_fence);

  const VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = _commandPool.getVkCommandPool(),
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1};

  vkAllocateCommandBuffers(device, &allocInfo, &_commandBuffer);

  const VkCommandBufferBeginInfo beginInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                              .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
  const LogicalDevice& logicalDevice = _commandPool.getLogicalDevice();
  const VkDevice device = logicalDevice.getVkDevice();

  vkEndCommandBuffer(_commandBuffer);

  const VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &_commandBuffer};

  vkQueueSubmit(logicalDevice.getVkQueue(_queueType), 1, &submitInfo, _fence);
  vkWaitForFences(device, 1, &_fence, VK_TRUE, UINT64_MAX);
  vkDestroyFence(device, _fence, nullptr);

  vkFreeCommandBuffers(device, _commandPool.getVkCommandPool(), 1, &_commandBuffer);
}

VkCommandBuffer SingleTimeCommandBuffer::getCommandBuffer() const noexcept {
  return _commandBuffer;
}
