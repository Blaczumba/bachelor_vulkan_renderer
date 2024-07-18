#include "texture_2D_sampler.h"

Texture2DSampler::Texture2DSampler(std::shared_ptr<LogicalDevice> logicalDevice, float samplerAnisotropy)
    : Texture2D(std::move(logicalDevice)), _samplerAnisotropy(samplerAnisotropy) {
}

Texture2DSampler::~Texture2DSampler() {
    VkDevice device = _logicalDevice->getVkDevice();
    
    vkDestroySampler(device, _sampler, nullptr);
    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}

const VkSampler Texture2DSampler::getVkSampler() const {
    return _sampler;
}
