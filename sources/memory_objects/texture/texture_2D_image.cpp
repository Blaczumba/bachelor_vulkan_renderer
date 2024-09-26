#include "texture_2D_image.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>
#include <stdexcept>

Texture2DImage::Texture2DImage(const LogicalDevice& logicalDevice, const std::string& texturePath, VkFormat format, float samplerAnisotropy)
	: Texture(
        Image{ 
            .format = format, .aspect = VK_IMAGE_ASPECT_COLOR_BIT, 
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        },
        Sampler{ 
            .maxAnisotropy = samplerAnisotropy
        }
    ),
    _logicalDevice(logicalDevice), _texturePath(texturePath) {

    const VkDevice device = _logicalDevice.getVkDevice();

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    _image.width              = static_cast<uint32_t>(texWidth);
    _image.height             = static_cast<uint32_t>(texHeight);
    _image.mipLevels          = std::floor(std::log2(std::max(_image.width, _image.height))) + 1u;
    _sampler.maxLod           = static_cast<float>(_image.mipLevels);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    _logicalDevice.createImage(_image.width, _image.height, _image.mipLevels, _image.numSamples, _image.format, _image.tiling, _image.usage, _image.properties, _image.image, _image.memory);
    
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(commandBuffer, stagingBuffer, _image.image, _image.width, _image.height);
        generateMipmaps(commandBuffer);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

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
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = _sampler.mipmapMode;
    samplerInfo.minLod = _sampler.minLod;
    samplerInfo.maxLod = _sampler.maxLod;
    samplerInfo.mipLodBias = _sampler.mipLodBias;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

Texture2DImage::~Texture2DImage() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroySampler(device, _sampler.sampler, nullptr);
    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
