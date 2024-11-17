#include "texture_2D_shadow.h"

#include "command_buffer/command_buffer.h"

Texture2DShadow::Texture2DShadow(const CommandPool& commandPool, const Image& image, const Sampler& sampler)
    : Texture(image, sampler), _logicalDevice(commandPool.getLogicalDevice()) {
    _logicalDevice.createImage(&_image);
    {
        SingleTimeCommandBuffer handle(commandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }    

    _logicalDevice.createImageView(&_image);
    _logicalDevice.createSampler(&_sampler);
}

Texture2DShadow::~Texture2DShadow() {
    const VkDevice device = _logicalDevice.getVkDevice();

    vkDestroySampler(device, _sampler.sampler, nullptr);
    vkDestroyImageView(device, _image.view, nullptr);
    vkDestroyImage(device, _image.image, nullptr);
    vkFreeMemory(device, _image.memory, nullptr);
}
