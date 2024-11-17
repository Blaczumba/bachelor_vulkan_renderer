#include "texture_2D_image.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>
#include <stdexcept>

Texture2DImage::Texture2DImage(const LogicalDevice& logicalDevice, std::string_view texturePath, const Image& image, const Sampler& sampler)
	: Texture(image, sampler), _logicalDevice(logicalDevice), _texturePath(texturePath) {
    const VkDevice device = _logicalDevice.getVkDevice();

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    _image.width              = static_cast<uint32_t>(texWidth);
    _image.height             = static_cast<uint32_t>(texHeight);
    _image.mipLevels          = std::floor(std::log2(std::max(_image.width, _image.height))) + 1u;
    _sampler.maxLod           = static_cast<float>(_image.mipLevels);

    VkDeviceSize imageSize = _image.width * _image.height * 4;

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

    _logicalDevice.createImage(&_image);
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(commandBuffer, stagingBuffer, _image.image, _image.width, _image.height);
        generateMipmaps(commandBuffer);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    _logicalDevice.createImageView(&_image);

    _logicalDevice.createSampler(&_sampler);
}

Texture2DImage::~Texture2DImage() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroySampler(device, _sampler.sampler, nullptr);
    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
