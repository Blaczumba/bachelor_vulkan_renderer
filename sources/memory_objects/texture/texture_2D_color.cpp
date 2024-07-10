#include "texture_2D_color.h"
#include "memory_objects/image.h"

#include <stdexcept>

Texture2DColor::Texture2DColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Texture2D(std::move(logicalDevice)) {

    _sampleCount    = samples;
    _image.format   = format;
    _image.extent   = { extent.width, extent.height, 1 };

    _logicalDevice->createImage(extent.width, extent.height, 1, samples, _image.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image.image, _image.memory);
    _image.view     = _logicalDevice->createImageView(_image.image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

Texture2DColor::~Texture2DColor() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}

void Texture2DColor::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayout(commandBuffer, _image.image, _image.layout, newLayout, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    _image.layout = newLayout;
}

