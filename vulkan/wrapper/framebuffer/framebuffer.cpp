#include "framebuffer.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/util/check.h"

Framebuffer Framebuffer::create(const Renderpass& renderpass, VkExtent2D extent,
                                std::span<const VkImageView> attachments) {
  const VkFramebufferCreateInfo framebufferInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = renderpass.getVkRenderPass(),
    .attachmentCount = static_cast<uint32_t>(attachments.size()),
    .pAttachments = attachments.data(),
    .width = extent.width,
    .height = extent.height,
    .layers = 1};

  VkFramebuffer framebuffer;
  CHECK_VKCMD(vkCreateFramebuffer(renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo,
                                  nullptr, &framebuffer),
              "Failed to create VkFramebuffer.");

  return Framebuffer(
      framebuffer, renderpass,
      VkViewport{.width = static_cast<float>(extent.width),
                 .height = static_cast<float>(extent.height),
                 .minDepth = 0.0f,
                 .maxDepth = 1.0f},
      VkRect2D{.extent = extent});
}

Framebuffer::Framebuffer(VkFramebuffer framebuffer, const Renderpass& renderpass,
                         const VkViewport& viewport, const VkRect2D& scissor) noexcept
  : _framebuffer(framebuffer), _renderpass(&renderpass), _viewport(viewport), _scissor(scissor) {}

Framebuffer::Framebuffer(Framebuffer&& framebuffer) noexcept
  : _framebuffer(std::exchange(framebuffer._framebuffer, VK_NULL_HANDLE)),
    _renderpass(std::exchange(framebuffer._renderpass, nullptr)), _viewport(framebuffer._viewport),
    _scissor(framebuffer._scissor) {}

void Framebuffer::destroy() {
  if (_framebuffer != VK_NULL_HANDLE) {
    _renderpass->getLogicalDevice().destroyResource(
        [framebuffer = _framebuffer](DestroyerContext context) {
          vkDestroyFramebuffer(context.device, framebuffer, context.allocationCallbacks);
        });
  }
}

Framebuffer& Framebuffer::operator=(Framebuffer&& framebuffer) noexcept {
  if (this == &framebuffer) {
    return *this;
  }

  destroy();

  _framebuffer = std::exchange(framebuffer._framebuffer, VK_NULL_HANDLE);
  _renderpass = std::exchange(framebuffer._renderpass, nullptr);
  _viewport = framebuffer._viewport;
  _scissor = framebuffer._scissor;
  return *this;
}

Framebuffer::~Framebuffer() {
  destroy();
}

VkExtent2D Framebuffer::getVkExtent() const noexcept {
  return _scissor.extent;
}

const VkViewport& Framebuffer::getViewport() const noexcept {
  return _viewport;
}

const VkRect2D& Framebuffer::getScissor() const noexcept {
  return _scissor;
}

const Renderpass& Framebuffer::getRenderpass() const {
  return *_renderpass;
}

VkFramebuffer Framebuffer::getVkFramebuffer() const noexcept {
  return _framebuffer;
}
