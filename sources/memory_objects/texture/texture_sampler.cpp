#include "texture_sampler.h"

TextureSampler::TextureSampler(float samplerAnisotropy) 
    : _samplerAnisotropy(samplerAnisotropy) {

}

const VkSampler TextureSampler::getVkSampler() const {
    return _sampler;
}