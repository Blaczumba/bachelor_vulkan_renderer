#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"

std::unique_ptr<Texture> createShadowMap(const CommandPool& commandPool, ImageParameters&& imageParams, SamplerParameters&& samplerParams) {
    const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();

    const VkImage image = logicalDevice.createImage(imageParams);
    const VkDeviceMemory memory = logicalDevice.createImageMemory(image, imageParams);
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    }
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    return std::make_unique<Texture>(logicalDevice, TextureType::SHADOWMAP, image, memory, imageParams, view, sampler, samplerParams);
}
