#include "image_color.h"

ImageColor::ImageColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : Image(format), _logicalDevice(logicalDevice) {

    _logicalDevice->createImage(extent.width, extent.height, 1, samples, _format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);

    _view = _logicalDevice->createImageView(_image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

}

ImageColor::~ImageColor() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
}