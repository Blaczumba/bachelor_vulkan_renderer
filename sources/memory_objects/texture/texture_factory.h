#pragma once

#include "texture.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string_view>

class CommandPool;

class TextureFactory {
public:
	static std::unique_ptr<Texture> createCubemap(const CommandPool& commandPool, std::string_view filePath, VkFormat format, float samplerAnisotropy);
	static std::unique_ptr<Texture> create2DShadowmap(const CommandPool& commandPool, uint32_t width, uint32_t height, VkFormat format);
	static std::unique_ptr<Texture> create2DTextureImage(const CommandPool& commandPool, std::string_view texturePath, VkFormat format, float samplerAnisotropy);
	static std::unique_ptr<Texture> createColorAttachment(const CommandPool& commandPool, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
	static std::unique_ptr<Texture> createDepthAttachment(const CommandPool& commandPool, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
};
