#include "texture.h"

#include "memory_objects/image.h"

Texture::Texture(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice) {
}

const Image& Texture::getImage() const {
	return _image;
}
