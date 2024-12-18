#pragma once

#include "texture.h"

#include "memory_objects/staging_buffer.h"
#include "model_loader/image_loader/image_loader.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <chrono>
#include <memory>
#include <stdexcept>

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