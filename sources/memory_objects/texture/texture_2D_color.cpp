#include "texture_2D_color.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DColor::Texture2DColor(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Texture2D(samples), _logicalDevice(logicalDevice) {

    setParameters(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_ASPECT_COLOR_BIT, 1u, 1u, extent.width, extent.height);
    _logicalDevice.createImage(_width, _height, _mipLevels, _sampleCount, _format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);
    _view = _logicalDevice.createImageView(_image, _format, _aspect, _mipLevels);

    {
        SingleTimeCommandBuffer handle(logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
}

Texture2DColor::~Texture2DColor() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
}
