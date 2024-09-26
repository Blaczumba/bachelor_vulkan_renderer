#pragma once

#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;

struct Texture {
protected:
	Image _image;
	Sampler _sampler;

public:
	Texture(const Image& image, const Sampler& sampler = {});
	virtual ~Texture() = default;

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	const Image& getImage() const;
	const Sampler& getSampler() const;

protected:
	void generateMipmaps(VkCommandBuffer commandBuffer);
};