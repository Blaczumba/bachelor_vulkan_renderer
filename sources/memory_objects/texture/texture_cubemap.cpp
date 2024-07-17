#include "texture_cubemap.h"
#include "command_buffer/command_buffer.h"

#include <ktx.h>
#include <ktxvulkan.h>

#include <stdexcept>

TextureCubemap::TextureCubemap(std::shared_ptr<LogicalDevice> logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy)
	: Texture2D(logicalDevice), _filePath(filePath), _samplerAnisotropy(samplerAnisotropy) {
	VkDevice device = _logicalDevice->getVkDevice();

	ktxResult result;
	ktxTexture* ktxTexture;

	_image.format = format;

	result = ktxTexture_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	if (result != KTX_SUCCESS) {
		throw std::runtime_error("failed to load ktx file");
	}

	_image.extent.width		= ktxTexture->baseWidth;
	_image.extent.height	= ktxTexture->baseHeight;
	_mipLevels				= ktxTexture->numLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	VkDeviceSize imageSize = ktxTextureSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	_logicalDevice->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, ktxTextureData, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t level = 0; level < _mipLevels; level++)
		{
			// Calculate offset into staging buffer for the current mip level and face
			ktx_size_t offset;
			KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
			if (result != KTX_SUCCESS) {
				throw std::runtime_error("failed to get image offset");
			}

			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = _image.extent.width >> level;
			bufferCopyRegion.imageExtent.height = _image.extent.height >> level;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;
			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = _mipLevels;
	subresourceRange.layerCount = 6;

	_logicalDevice->createImage(_image.extent.width, _image.extent.height, _mipLevels, VK_SAMPLE_COUNT_1_BIT, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory, 6);
	{
		SingleTimeCommandBuffer handle(_logicalDevice.get());
		VkCommandBuffer commandBuffer = handle.getCommandBuffer();
		transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(commandBuffer, stagingBuffer, _image.image, std::move(bufferCopyRegions));
		transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		//generateMipmaps(commandBuffer, _image.image, _image.format, _image.layout, _image.extent.width, _image.extent.height, _mipLevels);
	}

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	_image.view = _logicalDevice->createImageView(_image.image, _image.format, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels, 6);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = _samplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(_mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

TextureCubemap::~TextureCubemap() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkDestroySampler(device, _sampler, nullptr);
	vkDestroyImageView(device, _image.view, nullptr);
	vkDestroyImage(device, _image.image, nullptr);
	vkFreeMemory(device, _image.memory, nullptr);
}

void TextureCubemap::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
	transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels, 6);
	_image.layout = newLayout;
}
