#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct ImageDimensions {
	uint32_t width;
	uint32_t height;
	uint32_t mipLevels;
	uint32_t layerCount;
	std::vector<VkBufferImageCopy> copyRegions;
};

struct ImageResource {
	ImageDimensions dimensions;
	void* data;
	size_t size;
};

struct ImageParameters {
	VkFormat format = VK_FORMAT_UNDEFINED;
	uint32_t width = 1;
	uint32_t height = 1;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_NONE;
	uint32_t mipLevels = 1;
	VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage = 0;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t layerCount = 1;
};

struct SamplerParameters {
	VkFilter magFilter = VK_FILTER_LINEAR;
	VkFilter minFilter = VK_FILTER_LINEAR;
	VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	float mipLodBias = 0.0f;
	std::optional<float> maxAnisotropy = std::nullopt;
	std::optional<VkCompareOp> compareOp = std::nullopt;
	float minLod = 0.0f;
	float maxLod = 1.0f;
	VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	VkBool32 unnormalizedCoordinates = VK_FALSE;
};