#include "texture_2D_color.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DColor::Texture2DColor(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Texture(
        Image{
            .format = format, 
            .width = extent.width, 
            .height = extent.height, 
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT, 
            .mipLevels = 1, .numSamples = samples, 
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
        }
    ),
    _logicalDevice(logicalDevice) {

    _logicalDevice.createImage(_image.width, _image.height, _image.mipLevels, _image.numSamples, _image.format, _image.tiling, _image.usage, _image.properties, _image.image, _image.memory);
    _image.view = _logicalDevice.createImageView(_image.image, _image.format, _image.aspect, _image.mipLevels);

    {
        SingleTimeCommandBuffer handle(logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
}

Texture2DColor::~Texture2DColor() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
