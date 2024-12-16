#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"
#include "memory_objects/staging_buffer.h"

#include <ktxvulkan.h>
#include <ktx.h>

#include <cstring>
#include <stdexcept>

std::unique_ptr<Texture> createImageCubemap(const CommandPool& commandPool, std::string_view texturePath, ImageParameters&& imageParams, SamplerParameters&& samplerParams) {
	const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();
	const VkDevice device = logicalDevice.getVkDevice();

	const ImageLoader::Image imageBuffer = ImageLoader::loadCubemapImage(texturePath);
	imageParams.width = imageBuffer.width;
	imageParams.height = imageBuffer.height;
	imageParams.layerCount = imageBuffer.layerCount;
	imageParams.mipLevels = imageBuffer.mipLevels;
	samplerParams.maxLod = imageBuffer.mipLevels;

	const StagingBuffer stagingBuffer(logicalDevice.getMemoryAllocator(), std::span{ static_cast<uint8_t*>(imageBuffer.data), imageBuffer.size });

	std::free(imageBuffer.data);

	const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
	{
		SingleTimeCommandBuffer handle(commandPool);
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
		copyBufferToImage(commandBuffer, stagingBuffer.getBuffer().buffer, image, imageBuffer.copyRegions);
		transitionImageLayout(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
	}
	imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	const VkImageView view = logicalDevice.createImageView(image, imageParams);
	const VkSampler sampler = logicalDevice.createSampler(samplerParams);
	return std::make_unique<Texture>(logicalDevice, Texture::Type::CUBEMAP, image, nullptr, imageParams, view, sampler, samplerParams);
}
