#include "texture_2D_shadow.h"
#include "logical_device/logical_device.h"
#include "memory_objects/image.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DShadow::Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat depthFormat)
	: Texture2DSampler(std::move(logicalDevice), 1.0f) {
    const VkDevice device = _logicalDevice.getVkDevice();
    
    _image.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    _image.format = depthFormat;
    _image.extent = { width, height, 1 };

    _layerCount = 1;
    _mipLevels = 1;
    _sampleCount = VK_SAMPLE_COUNT_1_BIT;

    _logicalDevice.createImage(_image.extent.width, _image.extent.height, _mipLevels, _sampleCount, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory, _layerCount);
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    _image.view = _logicalDevice.createImageView(_image.image, _image.format, _image.aspect, _mipLevels, _layerCount);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = _samplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}