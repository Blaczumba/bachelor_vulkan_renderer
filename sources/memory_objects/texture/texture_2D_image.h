#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"
// #include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>
#include <memory>
#include <stdexcept>

std::unique_ptr<Texture> create2DImage(const CommandPool& commandPool, std::string_view texturePath, ImageParameters&& imageParams, SamplerParameters&& samplerParams) {
    const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();
    const VkDevice device = logicalDevice.getVkDevice();

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    imageParams.width = static_cast<uint32_t>(texWidth);
    imageParams.height = static_cast<uint32_t>(texHeight);
    imageParams.mipLevels = std::floor(std::log2(std::max(imageParams.width, imageParams.height))) + 1u;
    samplerParams.maxLod = static_cast<float>(imageParams.mipLevels);

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    const VkDeviceSize imageSize = imageParams.width * imageParams.height * 4;
    const VkBuffer stagingBuffer = logicalDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const VkDeviceMemory stagingBufferMemory = logicalDevice.createBufferMemory(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
        copyBufferToImage(commandBuffer, stagingBuffer, image, imageParams.width, imageParams.height);
        generateImageMipmaps(commandBuffer, image, imageParams.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.width, imageParams.height, imageParams.mipLevels, imageParams.layerCount);
    }
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    return std::make_unique<Texture>(logicalDevice, Texture::Type::IMAGE_2D, image, nullptr, imageParams, view, sampler, samplerParams);
}