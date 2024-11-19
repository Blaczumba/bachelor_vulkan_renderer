#pragma once

#include "memory_objects/buffers.h"

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;

enum class TextureType : uint8_t {
	IMAGE_2D,
	SHADOWMAP,
	COLOR_ATTACHMENT,
	DEPTH_ATTACHMENT,
	CUBEMAP
};

struct Texture {
	const TextureType _type;

	VkImage _image = VK_NULL_HANDLE;
	VkDeviceMemory _memory = VK_NULL_HANDLE;
	VkImageView _view = VK_NULL_HANDLE;
	VkSampler _sampler = VK_NULL_HANDLE;

	ImageParameters _imageParameters;
	SamplerParameters _samplerParameters;

	const LogicalDevice& _logicalDevice;
public:
	Texture(const LogicalDevice& logicalDevice, TextureType type, const VkImage image, const VkDeviceMemory memory, const ImageParameters& imageParameters, const VkImageView view = VK_NULL_HANDLE, const VkSampler sampler = VK_NULL_HANDLE, const SamplerParameters& samplerParameters = {});
	~Texture();

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	const VkImage getVkImage() const;
	const VkDeviceMemory getVkDeviceMemory() const;
	const VkImageView getVkImageView() const;
	const VkSampler getVkSampler() const;
	const ImageParameters& getImageParameters() const;
	const SamplerParameters& getSamplerParameters() const;
	VkExtent2D getVkExtent2D() const;

private:
	void generateMipmaps(VkCommandBuffer commandBuffer);
};