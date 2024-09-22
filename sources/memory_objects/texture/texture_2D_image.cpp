#include "texture_2D_image.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>
#include <stdexcept>

Texture2DImage::Texture2DImage(const LogicalDevice& logicalDevice, const std::string& texturePath, VkFormat format, float samplerAnisotropy)
	: Texture2D(VK_SAMPLE_COUNT_1_BIT), TextureSampler(samplerAnisotropy), _logicalDevice(logicalDevice), _texturePath(texturePath) {

    const VkDevice device = _logicalDevice.getVkDevice();

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    uint32_t width              = static_cast<uint32_t>(texWidth);
    uint32_t height             = static_cast<uint32_t>(texHeight);
    uint32_t mipLevels          = std::floor(std::log2(std::max(width, height))) + 1u;

    setParameters(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_ASPECT_COLOR_BIT, std::floor(std::log2(std::max(width, height))) + 1u, 1u, width, height);

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

    _logicalDevice.createImage(_width, _height, _mipLevels, _sampleCount, _format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);
    
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(commandBuffer, stagingBuffer, _image, _width, _height);
        generateMipmaps(commandBuffer);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    _view = _logicalDevice.createImageView(_image, _format, _aspect, _mipLevels);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    if(_samplerAnisotropy > 1.0f)
        samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = _samplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

Texture2DImage::~Texture2DImage() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroySampler(device, _sampler, nullptr);
    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
}
