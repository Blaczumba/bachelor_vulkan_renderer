#include "texture_2D_shadow.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DShadow::Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat format)
    : Texture(
        Image{ 
            .format = format, 
            .width = width, 
            .height = height, 
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT, 
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
        },
        Sampler{ 
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
            .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL, 
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE 
        }
    ), 
    _logicalDevice(logicalDevice) {
    const VkDevice device = _logicalDevice.getVkDevice();

    _logicalDevice.createImage(_image.width, _image.height, _image.mipLevels, _image.numSamples, _image.format, _image.tiling, _image.usage, _image.properties, _image.image, _image.memory);
    
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    _image.view = _logicalDevice.createImageView(_image.image, _image.format, _image.aspect, _image.mipLevels);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = _sampler.magFilter;
    samplerInfo.minFilter = _sampler.minFilter;
    samplerInfo.addressModeU = _sampler.addressModeU;
    samplerInfo.addressModeV = _sampler.addressModeV;
    samplerInfo.addressModeW = _sampler.addressModeW;
    if (_sampler.maxAnisotropy) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = _sampler.maxAnisotropy.value();
    }
    samplerInfo.borderColor = _sampler.borderColor;
    samplerInfo.unnormalizedCoordinates = _sampler.unnormalizedCoordinates;
    if (_sampler.compareOp) {
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = _sampler.compareOp.value();
    }
    else {
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    }
    samplerInfo.mipmapMode = _sampler.mipmapMode;
    samplerInfo.minLod = _sampler.minLod;
    samplerInfo.maxLod = _sampler.maxLod;
    samplerInfo.mipLodBias = _sampler.mipLodBias;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

Texture2DShadow::~Texture2DShadow() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroySampler(device, _sampler.sampler, nullptr);
    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
