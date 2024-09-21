#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

class TextureSampler {
protected:
	VkSampler _sampler;

public:
	virtual ~TextureSampler() {
		if (_sampler != VK_NULL_HANDLE)
			vkDestroySampler(logicalDevice, _sampler, nullptr);
	}

	const VkSampler getVkSampler() const {
		return _sampler;
	}
};
