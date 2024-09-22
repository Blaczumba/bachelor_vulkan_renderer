#include "texture_2D.h"
#include "memory_objects/image.h"
#include "command_buffer/command_buffer.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

Texture2D::Texture2D(VkSampleCountFlagBits sampleCount)
    : _sampleCount(sampleCount) {

}

VkExtent2D Texture2D::getVkExtent2D() const {
    return { _width, _height };
}
