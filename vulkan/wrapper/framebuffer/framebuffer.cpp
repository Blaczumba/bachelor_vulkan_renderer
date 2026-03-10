#include "framebuffer.h"

#include <cmath>
#include <optional>

#include "common/util/engine_exception.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/util/check.h"
#include "vulkan/wrapper/util/util.h"

namespace {

Texture createColorAttachment(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkFormat format,
    VkSampleCountFlagBits samples, VkExtent2D extent, uint32_t numLayers) {
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_COLOR_BIT)
          .withExtent(extent.width, extent.height)
          .withFormat(format)
          .withLayerCount(numLayers)
          .withNumSamples(samples)
          .withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
          .buildImage(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  texture.addCreateVkImageView(0, 1, 0, numLayers);
  return texture;
}

Texture createDepthAttachment(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkFormat format,
    VkSampleCountFlagBits samples, VkExtent2D extent, uint32_t numLayers) {
  Texture texture =
      TextureBuilder()
          .withAspect(hasStencil(format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
                                           VK_IMAGE_ASPECT_DEPTH_BIT)
          .withExtent(extent.width, extent.height)
          .withFormat(format)
          .withLayerCount(numLayers)
          .withNumSamples(samples)
          .withUsage(
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
          .buildImage(logicalDevice, commandBuffer,
                           hasStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
  texture.addCreateVkImageView(0, 1, 0, numLayers);
  return texture;
}

Texture createFragmentShadingRateAttachment(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkFormat format,
    VkSampleCountFlagBits samples, VkExtent2D extent, uint32_t numLayers) {
  const VkPhysicalDeviceFragmentShadingRatePropertiesKHR& fsrProperties =
      logicalDevice.getPhysicalDevice().getFragmentShadingRateProperties();
  const VkExtent2D fsrTexelExtent = fsrProperties.maxFragmentShadingRateAttachmentTexelSize;
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_COLOR_BIT)
          .withExtent(static_cast<uint32_t>(
                          std::ceil(extent.width / static_cast<float>(fsrTexelExtent.width))),
                      static_cast<uint32_t>(
                          std::ceil(extent.height / static_cast<float>(fsrTexelExtent.height))))
          .withFormat(format)
          .withLayerCount(numLayers)
          .withNumSamples(samples)
          .withUsage(VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
               | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
          .buildImage(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  texture.addCreateVkImageView(0, 1, 0, numLayers);
  return texture;
}

}  // namespace

Framebuffer Framebuffer::createFromSwapchain(
    VkCommandBuffer commandBuffer, const Renderpass& renderpass, VkExtent2D swapchainExtent,
    uint32_t numLayers, VkImageView swapchainImageView, std::vector<Texture>& attachments) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  std::span<const VkAttachmentDescription2> attachmentDescriptions =
      renderpass.getAttachmentsLayout().getVkAttachmentDescriptions();

  std::vector<VkImageView> imageViews;
  imageViews.reserve(attachmentDescriptions.size());
  for (const VkAttachmentDescription2& description : attachmentDescriptions) {
    switch (description.finalLayout) {
      case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        imageViews.push_back(swapchainImageView);
        break;
      case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        {
          Texture attachment = createColorAttachment(
              logicalDevice, commandBuffer, description.format, description.samples,
              swapchainExtent, numLayers);
          imageViews.push_back(attachment.getVkImageView());
          attachments.push_back(std::move(attachment));
          break;
        }
      case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        {
          Texture attachment = createDepthAttachment(
              logicalDevice, commandBuffer, description.format, description.samples,
              swapchainExtent, numLayers);
          imageViews.push_back(attachment.getVkImageView());
          attachments.push_back(std::move(attachment));
          break;
        }
      case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      case VK_IMAGE_LAYOUT_GENERAL:
        {
          Texture attachment = createFragmentShadingRateAttachment(
              logicalDevice, commandBuffer, description.format, description.samples,
              swapchainExtent, numLayers);
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
    .layers = 1};

  VkFramebuffer framebuffer;
  CHECK_VKCMD(vkCreateFramebuffer(renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo,
                                  nullptr, &framebuffer),
              "Failed to create VkFramebuffer.");

  return Framebuffer(
      framebuffer, renderpass,
      VkViewport{.width = static_cast<float>(swapchainExtent.width),
                 .height = static_cast<float>(swapchainExtent.height),
                 .minDepth = 0.0f,
                 .maxDepth = 1.0f},
      VkRect2D{.extent = swapchainExtent});
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

  return Framebuffer(
      framebuffer, renderpass,
      VkViewport{.width = static_cast<float>(extent->width),
                 .height = static_cast<float>(extent->height),
                 .minDepth = 0.0f,
                 .maxDepth = 1.0f},
      VkRect2D{.extent = *extent});
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
