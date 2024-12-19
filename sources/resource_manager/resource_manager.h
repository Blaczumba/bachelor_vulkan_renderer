#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/index_buffer.h"
#include "memory_objects/staging_buffer.h"

#include <unordered_map>
#include <string_view>

class ResourceManager {
public:
	ResourceManager(const LogicalDevice& logicalDevice);
	std::optional<std::reference_wrapper<Texture>> getTexture(std::string_view filePath);
	Texture& create2DTexture(std::string_view filePath, const VkCommandBuffer commandBuffer, VkFormat format, float samplerAnisotropy, const StagingBuffer& stagingBuffer, const ImageDimensions& imageDimensions);
	Texture& createCubemap(std::string_view filePath, const VkCommandBuffer commandBuffer, VkFormat format, float samplerAnisotropy, const StagingBuffer& stagingBuffer, const ImageDimensions& imageDimensions);
	Texture& create2DShadowMap(std::string_view filePath, const VkCommandBuffer commandBuffer, VkFormat format, uint32_t width, uint32_t height);
private:
	const LogicalDevice& _logicalDevice;

	std::unordered_map<std::string, std::unique_ptr<Texture>> _textures;	// TODO unordered_node_map
	std::unordered_map<std::string, IndexBuffer> _uniformBuffers; // TODO unordered_node_map
};
