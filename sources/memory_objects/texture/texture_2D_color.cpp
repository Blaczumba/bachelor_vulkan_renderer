#include "texture_2D_color.h"
#include "logical_device/logical_device.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DColor::Texture2DColor(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Texture2D(samples), _logicalDevice(logicalDevice) {

    VkImage image;
    VkDeviceMemory memory;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t mipLevels = 1u;

    _logicalDevice.createImage(extent.width, extent.height, mipLevels, samples, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
    _view = _logicalDevice.createImageView(image, format, aspect, mipLevels);
    
    _image = image;
    _memory = memory;
    _format = format;
    _aspect = aspect;
    _width = extent.width;
    _height = extent.height;
    _depth = 1u;
    _mipLevels = mipLevels;

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
