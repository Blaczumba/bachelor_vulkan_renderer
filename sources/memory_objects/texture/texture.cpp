#include "texture.h"

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
	transitionImageLayout(commandBuffer, _image, _layout, newLayout, _aspect, _mipLevels, _layerCount);
	_layout = newLayout;
}

void Texture::generateMipmaps(VkCommandBuffer commandBuffer) {
    _layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ::generateMipmaps(commandBuffer, _image, _format, _layout, _width, _height, _mipLevels, _layerCount);
}