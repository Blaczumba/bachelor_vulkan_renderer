#include "texture.h"

#include "logical_device/logical_device.h"
#include "memory_objects/buffers.h"

Texture::Texture(const LogicalDevice& logicalDevice, Texture::Type type, const VkImage image, const VkDeviceMemory memory, const ImageParameters& imageParameters, const VkImageView view, const VkSampler sampler, const SamplerParameters& samplerParameters)
    : _logicalDevice(logicalDevice), _type(type), _image(image), _memory(memory), _imageParameters(imageParameters), _view(view), _sampler(sampler), _samplerParameters(samplerParameters) {

}

Texture::~Texture() {
    const VkDevice device = _logicalDevice.getVkDevice();
    if(_sampler)
        vkDestroySampler(device, _sampler, nullptr);
    if(_view)
        vkDestroyImageView(device, _view, nullptr);
    if(_image)
        vkDestroyImage(device, _image, nullptr);
    if(_memory)
        vkFreeMemory(device, _memory, nullptr);
}

const VkImage Texture::getVkImage() const {
    return _image;
}

const VkDeviceMemory Texture::getVkDeviceMemory() const {
    return _memory;
}

const VkImageView Texture::getVkImageView() const {
    return _view;
}

const VkSampler Texture::getVkSampler() const {
    return _sampler;
}

VkExtent2D Texture::getVkExtent2D() const {
    return VkExtent2D{ _imageParameters.width, _imageParameters.height };
}

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, _image, _imageParameters.layout, newLayout, _imageParameters.aspect, _imageParameters.mipLevels, _imageParameters.layerCount);
}


void Texture::generateMipmaps(VkCommandBuffer commandBuffer) {
    const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    generateImageMipmaps(commandBuffer, _image, _imageParameters.format, finalLayout, _imageParameters.width, _imageParameters.height, _imageParameters.mipLevels, _imageParameters.layerCount);
    _imageParameters.layout = finalLayout;
}

const ImageParameters& Texture::getImageParameters() const {
    return _imageParameters;
}

const SamplerParameters& Texture::getSamplerParameters() const {
    return _samplerParameters;
}
