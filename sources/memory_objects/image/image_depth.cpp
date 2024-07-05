#include "image_depth.h"

ImageDepth::ImageDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent)
    : _logicalDevice(logicalDevice) {

    _logicalDevice->createImage(extent.width, extent.height, 1, samples, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);

    _view = _logicalDevice->createImageView(_image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

ImageDepth::~ImageDepth() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyImageView(device, _view, nullptr);
    vkDestroyImage(device, _image, nullptr);
    vkFreeMemory(device, _memory, nullptr);
}