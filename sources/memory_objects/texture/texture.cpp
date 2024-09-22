#include "texture.h"

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
	transitionImageLayout(commandBuffer, _image, _layout, newLayout, _aspect, _mipLevels, _layerCount);
	_layout = newLayout;
}

void Texture::setParameters(VkFormat format, VkImageLayout layout, VkImageAspectFlags aspect, uint32_t mipLevels, uint32_t layerCount, uint32_t width, uint32_t height, uint32_t depth) {
	_format = format;
	_layout = layout;
	_aspect = aspect;
	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = mipLevels;
	_layerCount = layerCount;
}

void Texture::generateMipmaps(VkCommandBuffer commandBuffer) {
    _layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ::generateMipmaps(commandBuffer, _image, _format, _layout, _width, _height, _mipLevels, _layerCount);
}