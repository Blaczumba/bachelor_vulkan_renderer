#pragma once

#include "texture.h"

#include "logical_device/logical_device.h"
#include "memory_objects/staging_buffer.h"

#include <ktxvulkan.h>
#include <ktx.h>

#include <cstring>
#include <stdexcept>

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
