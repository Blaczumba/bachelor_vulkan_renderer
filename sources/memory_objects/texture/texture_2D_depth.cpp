#include "texture_2D_depth.h"
#include "memory_objects/image.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DDepth::Texture2DDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
	: Texture2D(std::move(logicalDevice)) {

    _aspect         = VK_IMAGE_ASPECT_DEPTH_BIT;

    _sampleCount    = samples;
    _image.format   = format;
    _image.extent   = { extent.width, extent.height, 1 };

    _logicalDevice->createImage(extent.width, extent.height, 1, samples, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory);
    
    if (hasStencil(format))
        _aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _image.view = _logicalDevice->createImageView(_image.image, format, _aspect, 1);

    {
        SingleTimeCommandBuffer handle(_logicalDevice.get());
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

Texture2DDepth::~Texture2DDepth() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}

void Texture2DDepth::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, _aspect, 1);
    _image.layout = newLayout;
}


bool Texture2DDepth::hasStencil(VkFormat format) const {
    std::vector<VkFormat> formats =
    {
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
    };
    return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}
