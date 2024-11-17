#include "texture_cubemap.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

#include <ktxvulkan.h>
#include <ktx.h>

#include <cstring>
#include <stdexcept>

TextureCubemap::TextureCubemap(const CommandPool& commandPool, std::string_view filePath, const Image& image, const Sampler& sampler)
	: Texture(image, sampler), _logicalDevice(commandPool.getLogicalDevice()), _filePath(filePath) {
	const VkDevice device = _logicalDevice.getVkDevice();

	ktxResult result;
	ktxTexture* ktxTexture;

	result = ktxTexture_CreateFromNamedFile(filePath.data(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
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

			VkBufferImageCopy bufferCopyRegion = {
				.bufferOffset = offset,
				.imageSubresource = {
					.aspectMask = _image.aspect,
					.mipLevel = level,
					.baseArrayLayer = face,
					.layerCount = 1
				},
				.imageExtent = {
					.width = _image.width >> level,
					.height = _image.height >> level,
					.depth = 1 
				},
			};

			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	_logicalDevice.createImage(&_image);

	{
		SingleTimeCommandBuffer handle(commandPool);
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionImageLayout(commandBuffer, &_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(commandBuffer, stagingBuffer, _image.image, bufferCopyRegions);
		transitionImageLayout(commandBuffer, &_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	_logicalDevice.createImageView(&_image);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	_logicalDevice.createSampler(&_sampler);
}

TextureCubemap::~TextureCubemap() {
	const VkDevice device = _logicalDevice.getVkDevice();

	vkDestroySampler(device, _sampler.sampler, nullptr);
	vkDestroyImageView(device, _image.view, nullptr);
	vkDestroyImage(device, _image.image, nullptr);
	vkFreeMemory(device, _image.memory, nullptr);
}
