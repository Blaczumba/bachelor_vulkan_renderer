#pragma once

#include "memory_allocator/memory_allocator.h"
#include "memory_objects/buffers.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>

class LogicalDevice;

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


struct Texture {
public:
	enum class Type : uint8_t {
		IMAGE_2D,
		SHADOWMAP,
		COLOR_ATTACHMENT,
		DEPTH_ATTACHMENT,
		CUBEMAP
	};

private:
	const Type _type;

	VkImage _image = VK_NULL_HANDLE;
	VkDeviceMemory _memory = VK_NULL_HANDLE;
	VkImageView _view = VK_NULL_HANDLE;
	VkSampler _sampler = VK_NULL_HANDLE;

	ImageParameters _imageParameters;
	SamplerParameters _samplerParameters;

	const LogicalDevice& _logicalDevice;
public:
	Texture(const LogicalDevice& logicalDevice, Texture::Type type, const VkImage image, const VkDeviceMemory memory, const ImageParameters& imageParameters, const VkImageView view = VK_NULL_HANDLE, const VkSampler sampler = VK_NULL_HANDLE, const SamplerParameters& samplerParameters = {});
	~Texture();

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	const VkImage getVkImage() const;
	const VkImageView getVkImageView() const;
	const VkSampler getVkSampler() const;
	const ImageParameters& getImageParameters() const;
	const SamplerParameters& getSamplerParameters() const;
	VkExtent2D getVkExtent2D() const;

private:
	void generateMipmaps(VkCommandBuffer commandBuffer);
};

struct ImageCreator {
	const ImageParameters& params;

	const VkImage operator()(VmaWrapper& allocator) {
		return allocator.createVkImage(params, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	}

	const VkImage operator()(auto) {
		return nullptr;
	}

};