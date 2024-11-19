#include "texture.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

std::unique_ptr<Texture> createAttachment(const CommandPool& commandPool, VkImageLayout dstLayout, Texture::Type type, ImageParameters&& imageParams) {
    const LogicalDevice& logicalDevice = commandPool.getLogicalDevice();

    const VkImage image = logicalDevice.createImage(imageParams);
    const VkDeviceMemory memory = logicalDevice.createImageMemory(image, imageParams);
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionImageLayout(commandBuffer, image, imageParams.layout, dstLayout, imageParams.aspect, imageParams.mipLevels, imageParams.layerCount);
    }
    imageParams.layout = dstLayout;
    
    const VkImageView view = logicalDevice.createImageView(image, imageParams);

    return std::make_unique<Texture>(logicalDevice, type, image, memory, imageParams, view);
}
