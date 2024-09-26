#include "texture_cubemap.h"
#include "logical_device/logical_device.h"
#include "memory_objects/image.h"
#include "command_buffer/command_buffer.h"

#include <ktx.h>
#include <ktxvulkan.h>

#include <stdexcept>
#include <cstring>

// format, VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u, VK_SAMPLE_COUNT_1_BIT, samplerAnisotropy, 6u

TextureCubemap::TextureCubemap(const LogicalDevice& logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy)
	: Texture(
		Image{ 
			.format = format, 
			.aspect = VK_IMAGE_ASPECT_COLOR_BIT, 
			.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			.layerCount = 6u 
		},
		Sampler{ 
			.maxAnisotropy = samplerAnisotropy 
		}
	),
	_logicalDevice(logicalDevice), _filePath(filePath) {
	const VkDevice device = _logicalDevice.getVkDevice();

	ktxResult result;
	ktxTexture* ktxTexture;

	result = ktxTexture_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	if (result != KTX_SUCCESS) {
		throw std::runtime_error("failed to load ktx file");
	}

	_image.width = ktxTexture->baseWidth;
	_image.height = ktxTexture->baseHeight;
	_image.mipLevels = ktxTexture->numLevels;
	_sampler.maxLod = _image.mipLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	VkDeviceSize imageSize = ktxTextureSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	_logicalDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, ktxTextureData, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < _image.layerCount; face++)
	{
		for (uint32_t level = 0; level < _image.mipLevels; level++)
		{
			// Calculate offset into staging buffer for the current mip level and face
			ktx_size_t offset;
			KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
			if (result != KTX_SUCCESS) {
				throw std::runtime_error("failed to get image offset");
			}

			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = _image.aspect;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = _image.width >> level;
			bufferCopyRegion.imageExtent.height = _image.height >> level;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;
			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	_logicalDevice.createImage(_image.width, _image.height, _image.mipLevels, _image.numSamples, _image.format, _image.tiling, _image.usage, _image.properties, _image.image, _image.memory, _image.layerCount);

	{
		SingleTimeCommandBuffer handle(_logicalDevice);
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionImageLayout(commandBuffer, &_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(commandBuffer, stagingBuffer, _image.image, std::move(bufferCopyRegions));
		transitionImageLayout(commandBuffer, &_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	_image.view = _logicalDevice.createImageView(_image.image, _image.format, _image.aspect, _image.mipLevels, _image.layerCount);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = _sampler.magFilter;
	samplerInfo.minFilter = _sampler.minFilter;
	samplerInfo.addressModeU = _sampler.addressModeU;
	samplerInfo.addressModeV = _sampler.addressModeV;
	samplerInfo.addressModeW = _sampler.addressModeW;
	if (_sampler.maxAnisotropy) {
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = _sampler.maxAnisotropy.value();
	}
	samplerInfo.borderColor = _sampler.borderColor;
	samplerInfo.unnormalizedCoordinates = _sampler.unnormalizedCoordinates;
	if (_sampler.compareOp) {
		samplerInfo.compareEnable = VK_TRUE;
		samplerInfo.compareOp = _sampler.compareOp.value();
	}
	else {
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	}
	samplerInfo.mipmapMode = _sampler.mipmapMode;
	samplerInfo.minLod = _sampler.minLod;
	samplerInfo.maxLod = _sampler.maxLod;
	samplerInfo.mipLodBias = _sampler.mipLodBias;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler.sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

TextureCubemap::~TextureCubemap() {
	const VkDevice device = _logicalDevice.getVkDevice();

	vkDestroySampler(device, _sampler.sampler, nullptr);
	vkDestroyImageView(device, _image.view, nullptr);
	vkDestroyImage(device, _image.image, nullptr);
	vkFreeMemory(device, _image.memory, nullptr);
}
