#include "framebuffer.h"

#include <optional>

#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/util/check.h"

namespace {

Texture createColorAttachment(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                              VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_COLOR_BIT)
          .withExtent(extent.width, extent.height)
          .withFormat(format)
          .withNumSamples(samples)
          .withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
          .withLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
          .buildAttachment(logicalDevice, commandBuffer);
  texture.addCreateVkImageView(0, 1, 0, 1);
  return texture;
}

bool hasStencil(VkFormat format) {
  static constexpr VkFormat formats[] = {VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                         VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
  return std::find(std::cbegin(formats), std::cend(formats), format) != std::cend(formats);
}

Texture createDepthAttachment(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                              VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
  Texture texture =
      TextureBuilder()
          .withAspect(hasStencil(format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
                                           VK_IMAGE_ASPECT_DEPTH_BIT)
          .withExtent(extent.width, extent.height)
          .withFormat(format)
          .withNumSamples(samples)
          .withUsage(
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
          .withLayout(hasStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          .buildAttachment(logicalDevice, commandBuffer);
  texture.addCreateVkImageView(0, 1, 0, 1);
  return texture;
}

}  // namespace

Framebuffer Framebuffer::createFromSwapchain(
    VkCommandBuffer commandBuffer, const Renderpass& renderpass, VkExtent2D swapchainExtent,
    VkImageView swapchainImageView, std::vector<Texture>& attachments) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  std::span<const VkAttachmentDescription> attachmentDescriptions =
      renderpass.getAttachmentsLayout().getVkAttachmentDescriptions();

  std::vector<VkImageView> imageViews;
  imageViews.reserve(attachmentDescriptions.size());
  for (const VkAttachmentDescription& description : attachmentDescriptions) {
    switch (description.finalLayout) {
      case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        imageViews.push_back(swapchainImageView);
        break;
      case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        {
          Texture attachment = createColorAttachment(
              logicalDevice, commandBuffer, description.format, description.samples,
              swapchainExtent);
          imageViews.push_back(attachment.getVkImageView());
          attachments.push_back(std::move(attachment));
          break;
        }
      case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        {
          Texture attachment = createDepthAttachment(
              logicalDevice, commandBuffer, description.format, description.samples,
              swapchainExtent);
          imageViews.push_back(attachment.getVkImageView());
          attachments.push_back(std::move(attachment));
          break;
        }
      default:
        throw EngineException("Not recognized type of VkImageLayout during Framebuffer creation.");
    }
  }

  const VkFramebufferCreateInfo framebufferInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = renderpass.getVkRenderPass(),
    .attachmentCount = static_cast<uint32_t>(imageViews.size()),
    .pAttachments = imageViews.data(),
    .width = swapchainExtent.width,
    .height = swapchainExtent.height,
    .layers = 1,
  };

  VkFramebuffer framebuffer;
  CHECK_VKCMD(vkCreateFramebuffer(renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo,
                                  nullptr, &framebuffer),
              "Failed to create VkFramebuffer.");

  const VkViewport viewport = {
    .width = static_cast<float>(swapchainExtent.width),
    .height = static_cast<float>(swapchainExtent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f};
  const VkRect2D scissor = {.extent = swapchainExtent};
  return Framebuffer(framebuffer, renderpass, viewport, scissor);
}

Framebuffer Framebuffer::createFromTextures(
    const Renderpass& renderpass, std::span<const Texture> textures) {
  std::vector<VkImageView> imageViews;
  imageViews.reserve(textures.size());
  std::optional<VkExtent2D> extent;
  for (const Texture& texture : textures) {
    imageViews.push_back(texture.getVkImageView());
    if (!extent.has_value()) {
      extent = texture.getVkExtent2D();
    } else if (VkExtent2D tmpExtent = texture.getVkExtent2D();
               extent->width != tmpExtent.width || extent->height != tmpExtent.height) {
      throw EngineException("All images must have the same size to create a Framebuffer.");
    }
  }

  if (!extent.has_value()) {
    throw EngineException("Framebuffer must have an attachment.");
  }

  const VkFramebufferCreateInfo framebufferInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = renderpass.getVkRenderPass(),
    .attachmentCount = static_cast<uint32_t>(imageViews.size()),
    .pAttachments = imageViews.data(),
    .width = extent->width,
    .height = extent->height,
    .layers = 1,
  };

  VkFramebuffer framebuffer;
  CHECK_VKCMD(vkCreateFramebuffer(renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo,
                                  nullptr, &framebuffer),
              "Failed to create VkFramebuffer.");

  const VkViewport viewport = {
    .width = static_cast<float>(extent->width),
    .height = static_cast<float>(extent->height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f};

  const VkRect2D scissor = {.extent = *extent};
  return Framebuffer(framebuffer, renderpass, viewport, scissor);
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
