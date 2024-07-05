#include "texture.h"

#include "memory_objects/image.h"

Texture::Texture(std::shared_ptr<LogicalDevice> logicalDevice) 
	: _logicalDevice(logicalDevice) {
}

VkImageLayout Texture::getImageLayout() const { 
	return _layout; 
}