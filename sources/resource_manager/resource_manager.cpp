#include "resource_manager.h"

#include "memory_objects/texture/image.h"
#include "memory_objects/texture/texture_2D_image.h"
#include "memory_objects/texture/texture_cubemap.h"
#include "memory_objects/texture/texture_2D_shadow.h"
#include "memory_objects/vertex_buffer.h"

ResourceManager::ResourceManager(const LogicalDevice& logicalDevice)
    : _logicalDevice(logicalDevice) {}

std::optional<std::reference_wrapper<Texture>> ResourceManager::getTexture(const std::string& filePath) {
    auto it = _textures.find(filePath);
    return it == _textures.cend() ? std::nullopt : std::make_optional(std::ref(*it->second));
}

Texture& ResourceManager::create2DTexture(
    const std::string& filePath,
    const VkCommandBuffer commandBuffer,
    VkFormat format,
    float samplerAnisotropy,
    const StagingBuffer& stagingBuffer,
    const ImageDimensions& imageDimensions) {

    ImageParameters imageParams = {
        .format = format,
        .width = imageDimensions.width,
        .height = imageDimensions.height,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevels = imageDimensions.mipLevels,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .layerCount = 1,
    };

    const SamplerParameters samplerParams = {
        .maxAnisotropy = samplerAnisotropy,
        .maxLod = static_cast<float>(imageDimensions.mipLevels),
    };

    auto result = _textures.emplace(filePath, create2DImage(_logicalDevice, commandBuffer, stagingBuffer, imageParams, samplerParams));
    return *result.first->second;
}

Texture& ResourceManager::createCubemap(
    const std::string& filePath,
    const VkCommandBuffer commandBuffer,
    VkFormat format,
    float samplerAnisotropy,
    const StagingBuffer& stagingBuffer,
    const ImageDimensions& imageDimensions) {
 
    ImageParameters imageParams = {
        .format = format,
        .width = imageDimensions.width,
        .height = imageDimensions.height,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevels = imageDimensions.mipLevels,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .layerCount = 6U,
    };
    const SamplerParameters samplerParams = {
        .maxAnisotropy = samplerAnisotropy,
        .maxLod = static_cast<float>(imageDimensions.mipLevels)
    };
 
    auto result = _textures.emplace(filePath, createImageCubemap(_logicalDevice, commandBuffer, stagingBuffer, imageParams, samplerParams));
    return *result.first->second;
}

Texture& ResourceManager::create2DShadowMap(const std::string& filePath, const VkCommandBuffer commandBuffer, VkFormat format, uint32_t width, uint32_t height) {
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
    auto result = _textures.emplace(filePath, createShadowMap(_logicalDevice, commandBuffer, imageParams, samplerParams));
    return *result.first->second;
}
