#include "logical_device/logical_device.h"

#include "texture.h"

Texture::Texture(const LogicalDevice& logicalDevice)
	: _logicalDevice(logicalDevice) {
}

const Image& Texture::getImage() const {
	return _image;
}

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
	transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, _image.aspect, _mipLevels, _layerCount);
	_image.layout = newLayout;
}