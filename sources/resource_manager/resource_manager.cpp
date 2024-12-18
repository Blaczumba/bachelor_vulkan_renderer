#include "resource_manager.h"

#include "memory_objects/texture/image.h"
#include "memory_objects/texture/texture_2D_image.h"
#include "memory_objects/texture/texture_cubemap.h"
#include "memory_objects/vertex_buffer.h"

ResourceManager::ResourceManager(const LogicalDevice& logicalDevice)
    : _logicalDevice(logicalDevice) {}

std::optional<std::reference_wrapper<Texture>> ResourceManager::getTexture(const std::string& filePath) {
    auto it = _textures.find(filePath);
    return it == _textures.cend() ? std::nullopt : std::make_optional(std::ref(*it->second));
}

//Texture& ResourceManager::create2DTexture(
//    const std::string& filePath,
//    const VkCommandBuffer commandBuffer,
//    VkFormat format,
//    float samplerAnisotropy,
//    const StagingBuffer& stagingBuffer,
//    const ImageDimensions& imageDimensions) {
//
//    ImageParameters imageParams = {
//        .format = format,
//        .width = imageDimensions.width,
//        .height = imageDimensions.height,
//        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
//        .mipLevels = imageDimensions.mipLevels,
//        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//        .layerCount = 1,
//    };
//
//    const SamplerParameters samplerParams = {
//        .maxAnisotropy = samplerAnisotropy,
//        .maxLod = static_cast<float>(imageDimensions.mipLevels),
//    };
//
//    auto texture = create2DImage(_logicalDevice, commandBuffer, stagingBuffer, imageParams, samplerParams);
//    auto result = _textures.emplace(filePath, std::move(texture));
//    return *result.first->second;
//}
//
//Texture& ResourceManager::createCubemap(
//    const std::string& filePath,
//    const VkCommandBuffer commandBuffer,
//    VkFormat format,
//    float samplerAnisotropy,
//    const StagingBuffer& stagingBuffer,
//    const ImageDimensions& imageDimensions) {
// 
//    ImageParameters imageParams = {
//        .format = format,
//        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
//        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//        .layerCount = 6U,
//    };
// 
//    const SamplerParameters samplerParams = {
//        .maxAnisotropy = samplerAnisotropy,
//    };
// 
//    auto texture = createImageCubemap(_logicalDevice, commandBuffer, stagingBuffer, imageParams, samplerParams);
//    auto result = _textures.emplace(filePath, std::move(texture));
//    return *result.first->second;
//}

