#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"

std::unique_ptr<Texture> createShadowMap(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, ImageParameters& imageParams, const SamplerParameters& samplerParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    const VkSampler sampler = logicalDevice.createSampler(samplerParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return std::make_unique<Texture>(logicalDevice, Texture::Type::SHADOWMAP, image, nullptr, imageParams, view, sampler, samplerParams);
}
