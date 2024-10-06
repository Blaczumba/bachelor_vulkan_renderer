#include "texture_2D_depth.h"

#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <algorithm>
#include <stdexcept>

Texture2DDepth::Texture2DDepth(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
	: Texture(
        Image{ 
            .format = format,
            .width = extent.width,
            .height = extent.height,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
            .numSamples = samples,
            .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        }
    ),
    _logicalDevice(logicalDevice) {

    _logicalDevice.createImage(&_image);
    if (hasStencil(format))
        _image.aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _logicalDevice.createImageView(&_image);

    {
        SingleTimeCommandBuffer handle(_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

Texture2DDepth::~Texture2DDepth() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
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
