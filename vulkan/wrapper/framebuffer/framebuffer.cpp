#include "framebuffer.h"

#include <cstdint>
#include <utility>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

Framebuffer::Framebuffer(const Renderpass& renderpass, VkFramebuffer framebuffer,
                         const VkViewport& viewport, const VkRect2D& scissor) noexcept
  : _framebuffer(framebuffer), _renderpass(&renderpass), _viewport(viewport), _scissor(scissor) {}

Framebuffer::Framebuffer(Framebuffer&& framebuffer) noexcept
  : _framebuffer(std::exchange(framebuffer._framebuffer, VK_NULL_HANDLE)),
    _renderpass(framebuffer._renderpass), _viewport(framebuffer._viewport),
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
  if (&framebuffer == this) {
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

Framebuffer Framebuffer::create(
    const Renderpass& renderpass, const VkFramebufferCreateInfo& createInfo) {
  VkFramebuffer framebuffer;
  CHECK_VKCMD(vkCreateFramebuffer(
                  renderpass.getLogicalDevice().getVkDevice(), &createInfo, nullptr, &framebuffer),
              "Failed to create VkFramebuffer.");

  return Framebuffer(
      renderpass, framebuffer,
      VkViewport{
        .width = static_cast<float>(createInfo.width),
        .height = static_cast<float>(createInfo.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
  },
      VkRect2D{.extent = {createInfo.width, createInfo.height}});
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

FramebufferBuilder& FramebufferBuilder::addAttachment(VkImageView attachment) {
  _metadata.attachments.push_back(attachment);
  return *this;
}

FramebufferBuilder& FramebufferBuilder::withExtent(VkExtent2D extent) noexcept {
  _metadata.extent = extent;
  return *this;
}

FramebufferBuilder& FramebufferBuilder::withLayers(uint32_t layers) noexcept {
  _metadata.layers = layers;
  return *this;
}

FramebufferBuilder& FramebufferBuilder::withFlags(VkFramebufferCreateFlags flags) noexcept {
  _metadata.flags = flags;
  return *this;
}

FramebufferMetadata FramebufferBuilder::getMetadata() const noexcept {
  return _metadata;
}

Framebuffer FramebufferBuilder::build(const Renderpass& renderpass) const {
  const VkFramebufferCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .flags = _metadata.flags,
    .renderPass = renderpass.getVkRenderPass(),
    .attachmentCount = static_cast<uint32_t>(_metadata.attachments.size()),
    .pAttachments = _metadata.attachments.data(),
    .width = _metadata.extent.width,
    .height = _metadata.extent.height,
    .layers = _metadata.layers};
  return Framebuffer::create(renderpass, createInfo);
}
