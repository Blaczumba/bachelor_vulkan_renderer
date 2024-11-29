#pragma once

#include "texture.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

#include <ktxvulkan.h>
#include <ktx.h>

#include <cstring>
#include <stdexcept>

std::unique_ptr<Texture> createIamgeCubemap(const CommandPool& commandPool, std::string_view texturePath, ImageParameters&& imageParams, SamplerParameters&& samplerParams) {
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

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	const VkDeviceSize imageSize = ktxTextureSize;
	const VkBuffer stagingBuffer = logicalDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const VkDeviceMemory stagingBufferMemory = logicalDevice.createBufferMemory(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, ktxTextureData, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < imageParams.layerCount; face++)
	{
		for (uint32_t level = 0; level < imageParams.mipLevels; level++)
		{
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

	const VkImage image = logicalDevice.createImage(imageParams);
	const VkDeviceMemory memory = logicalDevice.createImageMemory(image, imageParams);
	{
		SingleTimeCommandBuffer handle(commandPool);
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionImageLayout(commandBuffer, image, imageParams.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
		copyBufferToImage(commandBuffer, stagingBuffer, image, bufferCopyRegions);
		transitionImageLayout(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
	}
	imageParams.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	const VkImageView view = logicalDevice.createImageView(image, imageParams);
	const VkSampler sampler = logicalDevice.createSampler(samplerParams);
	return std::make_unique<Texture>(logicalDevice, Texture::Type::CUBEMAP, image, memory, imageParams, view, sampler, samplerParams);
}
