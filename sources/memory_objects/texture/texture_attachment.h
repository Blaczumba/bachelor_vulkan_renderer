#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

std::unique_ptr<Texture> createAttachment(const LogicalDevice& logicalDevice, const VkCommandBuffer commandBuffer, VkImageLayout dstLayout, Texture::Type type, ImageParameters&& imageParams) {
    const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
    const VkImageView view = logicalDevice.createImageView(image, imageParams);
    transitionImageLayout(commandBuffer, image, imageParams.layout, dstLayout, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    imageParams.layout = dstLayout;
    return std::make_unique<Texture>(logicalDevice, type, image, nullptr, imageParams, view);
}
