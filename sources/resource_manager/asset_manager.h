#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/staging_buffer.h"
#include "model_loader/image_loader/image_loader.h"
#include "thread_pool/thread_pool.h"

#include <mutex>
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
	AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool);
	CacheCode loadImage2D(std::string_view filePath);
	const std::pair<StagingBuffer, ImageDimensions>& getImageData(const std::string& filePath) const;
	CacheCode deleteImage(std::string_view filePath);

private:
	MemoryAllocator& _memoryAllocator;
	ThreadPool* _threadPool;
	uint8_t _index;
	std::mutex _mutex;

	std::unordered_map<std::string, std::pair<StagingBuffer, ImageDimensions>> _imageResources;
};
