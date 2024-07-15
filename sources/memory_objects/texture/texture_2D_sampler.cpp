#include "texture_2D_sampler.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <iostream>
#include <stdexcept>

Texture2DSampler::Texture2DSampler(std::shared_ptr<LogicalDevice> logicalDevice, const std::string& texturePath, float samplerAnisotropy)
    : Texture2D(std::move(logicalDevice)), _texturePath(texturePath), _samplerAnisotropy(samplerAnisotropy) {
    VkDevice device = _logicalDevice->getVkDevice();

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    _image.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
    _mipLevels = std::floor(std::log2(std::max(_image.extent.width, _image.extent.height))) + 1;

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _logicalDevice->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    _image.format = VK_FORMAT_R8G8B8A8_SRGB;

    _logicalDevice->createImage(_image.extent.width, _image.extent.height, _mipLevels, VK_SAMPLE_COUNT_1_BIT, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory);
    {
        SingleTimeCommandBuffer handle(_logicalDevice.get());
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(commandBuffer, stagingBuffer, _image.image, _image.extent.width, _image.extent.height);

        _image.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;   // It is set in generateMipmaps

        generateMipmaps(commandBuffer, _image.image, _image.format, _image.layout, _image.extent.width, _image.extent.height, _mipLevels);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    _image.view = _logicalDevice->createImageView(_image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = samplerAnisotropy;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias              = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
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

void Texture2DSampler::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
    _image.layout = newLayout;
}
