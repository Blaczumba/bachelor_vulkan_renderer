#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"
#include "memory_objects/staging_buffer.h"
#include "model_loader/image_loader/image_loader.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <chrono>
#include <memory>
#include <stdexcept>

std::unique_ptr<Texture> create2DImage(const CommandPool& commandPool, std::string_view texturePath, ImageParameters&& imageParams, SamplerParameters&& samplerParams) {
    const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();
    const VkDevice device = logicalDevice.getVkDevice();

    const ImageLoader::Image imageBuffer = ImageLoader::load2DImage(texturePath);
    imageParams.width = imageBuffer.width;
    imageParams.height = imageBuffer.height;
    imageParams.layerCount = imageBuffer.layerCount;
    imageParams.mipLevels = imageBuffer.mipLevels;

    const StagingBuffer stagingBuffer(logicalDevice.getMemoryAllocator(), std::span{ static_cast<uint8_t*>(imageBuffer.data), imageBuffer.size });

    std::free(imageBuffer.data);

    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
        copyBufferToImage(commandBuffer, stagingBuffer.getBuffer().buffer, image, imageBuffer.copyRegions);
        generateImageMipmaps(commandBuffer, image, imageParams.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.width, imageParams.height, imageParams.mipLevels, imageParams.layerCount);
    }
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    return std::make_unique<Texture>(logicalDevice, Texture::Type::IMAGE_2D, image, nullptr, imageParams, view, sampler, samplerParams);
}