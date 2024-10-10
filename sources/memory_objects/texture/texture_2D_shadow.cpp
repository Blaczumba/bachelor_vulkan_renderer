#include "texture_2D_shadow.h"

#include "command_buffer/command_buffer.h"
#include "logical_device/logical_device.h"

Texture2DShadow::Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat format)
    : Texture(
        Image{ 
            .format = format,
            .width = width,
            .height = height,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        },
        Sampler{ 
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
        }
    ), 
    _logicalDevice(logicalDevice) {
    const VkDevice device = _logicalDevice.getVkDevice();

    _logicalDevice.createImage(&_image);
    {
        SingleTimeCommandBuffer handle(_logicalDevice);
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
