#include "texture.h"

Texture::Texture(const Image& image, const Sampler& sampler) 
    : _image(image), _sampler(sampler) {

}

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, &_image, newLayout);
}


void Texture::generateMipmaps(VkCommandBuffer commandBuffer) {
    generateImageMipmaps(commandBuffer, &_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

const Image& Texture::getImage() const {
    return _image;
}


const Sampler& Texture::getSampler() const {
    return _sampler;
}
