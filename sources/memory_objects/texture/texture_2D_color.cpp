#include "texture_2D_color.h"
#include "memory_objects/image.h"
#include "command_buffer/command_buffer.h"

#include <stdexcept>

Texture2DColor::Texture2DColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Texture2D(std::move(logicalDevice)) {

    _aspect         = VK_IMAGE_ASPECT_COLOR_BIT;

    _sampleCount    = samples;
    _image.format   = format;
    _image.extent   = { extent.width, extent.height, 1 };

    _logicalDevice->createImage(extent.width, extent.height, 1, _sampleCount, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory);
    
    _image.view     = _logicalDevice->createImageView(_image.image, format, _aspect, 1);

    {
        SingleTimeCommandBuffer handle(_logicalDevice.get());
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
}

Texture2DColor::~Texture2DColor() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}

void Texture2DColor::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, _aspect, 1);
    _image.layout = newLayout;
}

