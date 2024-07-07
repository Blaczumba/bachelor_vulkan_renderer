#include "texture_2D_depth.h"
#include "memory_objects/image.h"

#include <stdexcept>

Texture2DDepth::Texture2DDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
	: Texture(std::move(logicalDevice), format) {

    _logicalDevice->createImage(extent.width, extent.height, 1, samples, _format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);
    // transitionImageLayout(_logicalDevice.get(), _textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    _view = _logicalDevice->createImageView(_image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(_logicalDevice->getVkDevice(), &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

Texture2DDepth::~Texture2DDepth() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroySampler(device, _sampler, nullptr);
    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
}
