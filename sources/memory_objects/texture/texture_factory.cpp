#include "texture_factory.h"

#include "logical_device/logical_device.h"
#include "memory_objects/staging_buffer.h"
#include "model_loader/image_loader/image_loader.h"

#include <array>

namespace {

std::unique_ptr<Texture> create2DImage(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, const StagingBuffer& stagingBuffer, ImageParameters& imageParams, const SamplerParameters& samplerParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    copyBufferToImage(commandBuffer, stagingBuffer.getBuffer().buffer, image, stagingBuffer.getImageCopyRegions());
    generateImageMipmaps(commandBuffer, image, imageParams.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.width, imageParams.height, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return std::make_unique<Texture>(logicalDevice, Texture::Type::IMAGE_2D, image, nullptr, imageParams, view, sampler, samplerParams);
}

std::unique_ptr<Texture> createShadowMap(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, ImageParameters& imageParams, const SamplerParameters& samplerParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return std::make_unique<Texture>(logicalDevice, Texture::Type::SHADOWMAP, image, nullptr, imageParams, view, sampler, samplerParams);
}

std::unique_ptr<Texture> createAttachment(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, VkImageLayout dstLayout, Texture::Type type, ImageParameters&& imageParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, dstLayout, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = dstLayout;
    return std::make_unique<Texture>(logicalDevice, type, image, nullptr, imageParams, view);
}

std::unique_ptr<Texture> createImageCubemap(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, const StagingBuffer& stagingBuffer, ImageParameters& imageParams, const SamplerParameters& samplerParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    copyBufferToImage(commandBuffer, stagingBuffer.getBuffer().buffer, image, stagingBuffer.getImageCopyRegions());
    transitionImageLayout(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return std::make_unique<Texture>(logicalDevice, Texture::Type::CUBEMAP, image, nullptr, imageParams, view, sampler, samplerParams);
}

}

std::unique_ptr<Texture> TextureFactory::createCubemap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, std::string_view texturePath, VkFormat format, float samplerAnisotropy) {
    const ImageResource imageBuffer = ImageLoader::loadCubemapImage(texturePath);
    ImageParameters imageParams = {
        .format = format,
        .width = imageBuffer.dimensions.width,
        .height = imageBuffer.dimensions.height,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevels = imageBuffer.dimensions.mipLevels,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .layerCount = 6U,
    };
    const SamplerParameters samplerParams = {
        .maxAnisotropy = samplerAnisotropy,
        .maxLod = static_cast<float>(imageBuffer.dimensions.mipLevels)
    };
    const StagingBuffer* stagingBuffer(new StagingBuffer(logicalDevice.getMemoryAllocator(), std::span{ static_cast<uint8_t*>(imageBuffer.data), imageBuffer.size }, imageBuffer.dimensions.copyRegions));
    std::free(imageBuffer.data);
    return createImageCubemap(logicalDevice, commandBuffer, *stagingBuffer, imageParams, samplerParams);
}

std::unique_ptr<Texture> TextureFactory::create2DShadowmap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkFormat format) {
    ImageParameters imageParams = {
        .format = format,
        .width = width,
        .height = height,
        .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    };
    const SamplerParameters samplerParams = {
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
    };
    return createShadowMap(logicalDevice, commandBuffer, imageParams, samplerParams);
}

std::unique_ptr<Texture> TextureFactory::create2DTextureImage(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, std::string_view texturePath, VkFormat format, float samplerAnisotropy) {
    const ImageResource imageBuffer = ImageLoader::load2DImage(texturePath);
    ImageParameters imageParams = {
        .format = format,
        .width = imageBuffer.dimensions.width,
        .height = imageBuffer.dimensions.height,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevels = imageBuffer.dimensions.mipLevels,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .layerCount = 1,
    };
    const SamplerParameters samplerParams = {
        .maxAnisotropy = samplerAnisotropy,
        .maxLod = static_cast<float>(imageBuffer.dimensions.mipLevels)
    };

    const StagingBuffer* stagingBuffer(new StagingBuffer(logicalDevice.getMemoryAllocator(), std::span{ static_cast<uint8_t*>(imageBuffer.data), imageBuffer.size }, imageBuffer.dimensions.copyRegions));

    std::free(imageBuffer.data);
    return create2DImage(logicalDevice, commandBuffer, *stagingBuffer, imageParams, samplerParams);
}

std::unique_ptr<Texture> TextureFactory::createColorAttachment(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
    return createAttachment(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Texture::Type::COLOR_ATTACHMENT,
        ImageParameters{
            .format = format,
            .width = extent.width,
            .height = extent.height,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevels = 1,
            .numSamples = samples,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        }
    );
}

bool hasStencil(VkFormat format) {
    const std::array<VkFormat, 4> formats = {
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };
    return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

std::unique_ptr<Texture> TextureFactory::createDepthAttachment(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
    const VkImageAspectFlags aspect = hasStencil(format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    return createAttachment(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, Texture::Type::DEPTH_ATTACHMENT,
        ImageParameters{
            .format = format,
            .width = extent.width,
            .height = extent.height,
            .aspect = aspect,
            .numSamples = samples,
            .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        }
    );
}
