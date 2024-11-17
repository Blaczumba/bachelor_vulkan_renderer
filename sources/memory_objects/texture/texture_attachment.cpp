#include "texture_attachment.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

TextureAttachment::TextureAttachment(const CommandPool& commandPool, VkImageLayout dstLayout, const Image& image)
    : Texture(image), _logicalDevice(commandPool.getLogicalDevice()) {

    _logicalDevice.createImage(&_image);
    _logicalDevice.createImageView(&_image);

    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();

        transitionLayout(commandBuffer, dstLayout);
    }
}

TextureAttachment::~TextureAttachment() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
