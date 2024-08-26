#include "framebuffer.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>


Framebuffer::Framebuffer(const LogicalDevice& logicaldevice, const Renderpass& renderpass)
    : _logicalDevice(logicaldevice), _renderPass(renderpass) {
    
}

const std::vector<std::shared_ptr<Texture2DColor>>& Framebuffer::getColorTextures() const {
    return _colorImages;
}

std::vector<VkFramebuffer> Framebuffer::getVkFramebuffers() const {
    return _framebuffers;
}
