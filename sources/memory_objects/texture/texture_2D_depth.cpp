#include "texture_2D_depth.h"

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <algorithm>
#include <stdexcept>

Texture2DDepth::Texture2DDepth(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
	: Texture2D(samples), _logicalDevice(logicalDevice) {

    VkImage image;
    VkDeviceMemory memory;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    uint32_t mipLevels = 1u;

    _logicalDevice.createImage(extent.width, extent.height, mipLevels, samples, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
    
    if (hasStencil(format))
        aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _view = _logicalDevice.createImageView(image, format, aspect, mipLevels);

    _image      = image;
    _memory     = memory;
    _format     = format;
    _aspect     = aspect;
    _width      = extent.width;
    _height     = extent.height;
    _depth      = 1u;
    _mipLevels  = mipLevels;

    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

Texture2DDepth::~Texture2DDepth() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
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
