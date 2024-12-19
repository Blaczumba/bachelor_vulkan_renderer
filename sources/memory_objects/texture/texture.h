#pragma once

#include "memory_allocator/memory_allocator.h"
#include "memory_allocator/allocation.h"
#include "memory_objects/buffers.h"
#include "memory_objects/texture/image.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>

class LogicalDevice;

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

	Allocation _allocation;
	VkImage _image = VK_NULL_HANDLE;
	VkImageView _view = VK_NULL_HANDLE;
	VkSampler _sampler = VK_NULL_HANDLE;

	ImageParameters _imageParameters;
	SamplerParameters _samplerParameters;

	const LogicalDevice& _logicalDevice;
public:
	Texture(const LogicalDevice& logicalDevice, Texture::Type type, const VkImage image, Allocation allocation, const ImageParameters& imageParameters, const VkImageView view = VK_NULL_HANDLE, const VkSampler sampler = VK_NULL_HANDLE, const SamplerParameters& samplerParameters = {});
	Texture(Texture&& texture) noexcept;
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
	Allocation& allocation;
	const ImageParameters& params;

	const VkImage operator()(VmaWrapper& allocator) {
		auto[image, tmpallocation] = allocator.createVkImage(params, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		allocation = tmpallocation;
		return image;
	}

	const VkImage operator()(auto&&) {
		return nullptr;
	}

};