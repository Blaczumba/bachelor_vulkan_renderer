#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/index_buffer.h"
#include "memory_objects/staging_buffer.h"

#include <unordered_map>
#include <string_view>

class ResourceManager {
public:
	ResourceManager(const LogicalDevice& logicalDevice);
	std::optional<std::reference_wrapper<Texture>> getTexture(const std::string& filePath);
	Texture& create2DTexture(const std::string& filePath, const VkCommandBuffer commandBuffer, VkFormat format, float samplerAnisotropy, const StagingBuffer& stagingBuffer, const ImageDimensions& imageDimensions);
	Texture& createCubemap(const std::string& filePath, const VkCommandBuffer commandBuffer, VkFormat format, float samplerAnisotropy, const StagingBuffer& stagingBuffer, const ImageDimensions& imageDimensions);

private:
	const LogicalDevice& _logicalDevice;

	std::unordered_map<std::string, std::unique_ptr<Texture>> _textures;	// TODO unordered_node_map
	std::unordered_map<std::string, IndexBuffer> _uniformBuffers; // TODO unordered_node_map
};
