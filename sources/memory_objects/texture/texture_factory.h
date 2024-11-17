#pragma once

#include "texture.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string_view>

class LogicalDevice;

class TextureFactory {
public:
	static std::unique_ptr<Texture> createCubemap(const LogicalDevice& logicalDevice, std::string_view filePath, VkFormat format, float samplerAnisotropy);
	static std::unique_ptr<Texture> create2DShadowmap(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat format);
	static std::unique_ptr<Texture> create2DTextureImage(const LogicalDevice& logicalDevice, std::string_view texturePath, VkFormat format, float samplerAnisotropy);
	static std::unique_ptr<Texture> createColorAttachment(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
	static std::unique_ptr<Texture> createDepthAttachment(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
};
