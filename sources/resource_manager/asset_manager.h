#pragma once

#include "model_loader/image_loader/image_loader.h"
#include "memory_objects/staging_buffer.h"
#include "logical_device/logical_device.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <span>

enum class CacheCode : uint8_t {
	NOT_CACHED,
	CACHED
};

class AssetManager {
public:
	AssetManager(MemoryAllocator& memoryAllocator);
	CacheCode loadImage2D(const std::string& filePath);
	const std::pair<StagingBuffer, ImageDimensions>& getImageData(const std::string& filePath) const;
	CacheCode deleteImage(const std::string& filePath);

private:
	MemoryAllocator& _memoryAllocator;

	std::unordered_map<std::string, std::pair<StagingBuffer, ImageDimensions>> _imageResources;
};
