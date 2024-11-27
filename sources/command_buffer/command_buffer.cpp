#include "command_buffer.h"

#include "framebuffer/framebuffer.h"

#include <stdexcept>

CommandPool::CommandPool(const LogicalDevice& logicalDevice) : _logicalDevice(logicalDevice) {
    const VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _logicalDevice.getPhysicalDevice().getPropertyManager().getQueueFamilyIndices().graphicsFamily.value()
    };

    if (vkCreateCommandPool(_logicalDevice.getVkDevice(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(_logicalDevice.getVkDevice(), _commandPool, nullptr);
}
std::unique_ptr<CommandBuffer> CommandPool::createPrimaryCommandBuffer() const {
    return std::make_unique<CommandBuffer>(*this, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

std::unique_ptr<CommandBuffer> CommandPool::createSecondaryCommandBuffer() const {
    return std::make_unique<CommandBuffer>(*this, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
}

const VkCommandPool CommandPool::getVkCommandPool() const {
    return _commandPool;
}

const LogicalDevice& CommandPool::getLogicalDevice() const {
    return _logicalDevice;
}

CommandBuffer::CommandBuffer(const CommandPool& commandPool, VkCommandBufferLevel level)
    :_commandPool(commandPool), _level(level) {
    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool.getVkCommandPool(),
        .level = level,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(_commandPool.getLogicalDevice().getVkDevice(), &allocInfo, &_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(_commandPool.getLogicalDevice().getVkDevice(), _commandPool.getVkCommandPool(), 1, &_commandBuffer);
}

VkResult CommandBuffer::begin(const Framebuffer& framebuffer, VkCommandBufferUsageFlags flags, uint32_t subpassIndex) const {
    const VkCommandBufferInheritanceInfo inheritance = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .renderPass = framebuffer.getRenderpass().getVkRenderPass(),
        .subpass = subpassIndex,
        .framebuffer = framebuffer.getVkFramebuffer()
    };
    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = flags,
        .pInheritanceInfo = &inheritance
    };

    return vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

VkResult CommandBuffer::submit(QueueType type, const VkSemaphore waitSemaphore, const VkSemaphore signalSemaphore, const VkFence waitFence) const {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { waitSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    if (waitSemaphore != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    VkSemaphore signalSemaphores[] = { signalSemaphore };
    if (signalSemaphore != VK_NULL_HANDLE) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }

    const LogicalDevice& logicalDevice = _commandPool.getLogicalDevice();
    if (waitFence != VK_NULL_HANDLE) {
        vkResetFences(logicalDevice.getVkDevice(), 1, &waitFence);
    }

    return vkQueueSubmit(logicalDevice.getQueue(type), 1, &submitInfo, waitFence);
}

void CommandBuffer::resetCommandBuffer() const {
    vkResetCommandBuffer(_commandBuffer, 0);
}

const VkCommandBuffer CommandBuffer::getVkCommandBuffer() const {
    return _commandBuffer;
}

SingleTimeCommandBuffer::SingleTimeCommandBuffer(const CommandPool& commandPool, QueueType queueType)
    : _commandPool(commandPool), _queueType(queueType) {
    const VkDevice device = _commandPool.getLogicalDevice().getVkDevice();
    const VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };

    if (vkCreateFence(device, &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create SingleTimeCommandBuffer fence!");
    }

    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool.getVkCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(device, &allocInfo, &_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
    const LogicalDevice& logicalDevice = _commandPool.getLogicalDevice();
    const VkDevice device = logicalDevice.getVkDevice();
    const VkQueue queue = logicalDevice.getQueue(_queueType);

    vkEndCommandBuffer(_commandBuffer);

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &_commandBuffer
    };

    vkQueueSubmit(queue, 1, &submitInfo, _fence);
    vkWaitForFences(device, 1, &_fence, VK_TRUE, UINT64_MAX);

    vkFreeCommandBuffers(device, _commandPool.getVkCommandPool(), 1, &_commandBuffer);
    vkDestroyFence(device, _fence, nullptr);
}

VkCommandBuffer SingleTimeCommandBuffer::getCommandBuffer() const {
    return _commandBuffer;
}
