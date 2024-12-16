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

	ktxResult result;
	ktxTexture* ktxTexture;

	result = ktxTexture_CreateFromNamedFile(texturePath.data(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	if (result != KTX_SUCCESS) {
		throw std::runtime_error("failed to load ktx file");
	}

	imageParams.width = ktxTexture->baseWidth;
	imageParams.height = ktxTexture->baseHeight;
	imageParams.mipLevels = samplerParams.maxLod = ktxTexture->numLevels;


	std::vector<VkBufferImageCopy> bufferCopyRegions;
	for (uint32_t face = 0; face < imageParams.layerCount; face++) {
		for (uint32_t level = 0; level < imageParams.mipLevels; level++) {
			// Calculate offset into staging buffer for the current mip level and face
			ktx_size_t offset;
			KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
			if (result != KTX_SUCCESS) {
				throw std::runtime_error("failed to get image offset");
			}

			const VkBufferImageCopy bufferCopyRegion = {
				.bufferOffset = offset,
				.imageSubresource = {
					.aspectMask = imageParams.aspect,
					.mipLevel = level,
					.baseArrayLayer = face,
					.layerCount = 1
				},
				.imageExtent = {
					.width = imageParams.width >> level,
					.height = imageParams.height >> level,
					.depth = 1
				},
			};

			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	const StagingBuffer stagingBuffer(logicalDevice.getMemoryAllocator(), std::span{ ktxTexture_GetData(ktxTexture), ktxTexture_GetSize(ktxTexture) });

	ktxTexture_Destroy(ktxTexture);

	const VkImage image = std::visit(ImageCreator{ imageParams }, logicalDevice.getMemoryAllocator());
	{
		SingleTimeCommandBuffer handle(commandPool);
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
		copyBufferToImage(commandBuffer, stagingBuffer.getBuffer().buffer, image, bufferCopyRegions);
		transitionImageLayout(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
	}
	imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	const VkImageView view = logicalDevice.createImageView(image, imageParams);
	const VkSampler sampler = logicalDevice.createSampler(samplerParams);
	return std::make_unique<Texture>(logicalDevice, Texture::Type::CUBEMAP, image, nullptr, imageParams, view, sampler, samplerParams);
}
