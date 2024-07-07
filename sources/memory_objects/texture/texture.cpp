#include "texture.h"

#include "memory_objects/image.h"

Texture::Texture(std::shared_ptr<LogicalDevice> logicalDevice) 
	: _logicalDevice(logicalDevice), _layout(VK_IMAGE_LAYOUT_UNDEFINED) {
}

Texture::~Texture() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkDestroySampler(device, _sampler, nullptr);
	vkDestroyImageView(device, _view, nullptr);
	vkDestroyImage(device, _image, nullptr);
	vkFreeMemory(device, _memory, nullptr);
}

VkImageLayout Texture::getVkImageLayout() const { 
	return _layout; 
}

VkSampler Texture::getVkSampler() const {
	return _sampler;
}