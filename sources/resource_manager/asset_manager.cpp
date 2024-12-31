#include "asset_manager.h"

#include <iostream>

AssetManager::AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool) : _memoryAllocator(memoryAllocator), _threadPool(threadPool), _index(0) {}

CacheCode AssetManager::loadImage2D(std::string_view filePath) {
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto it = _imageResources.find(std::string{ filePath });
		if (it != _imageResources.cend()) {
			return CacheCode::CACHED;
		}
	}
	if (_threadPool) {
		uint8_t index = _index.fetch_add(1);
		_threadPool->getThread(index % _threadPool->getNumThreads())->addJob([&, filePath = std::string{filePath}]() {
				ImageResource resource = ImageLoader::load2DImage(filePath);
				StagingBuffer stagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size));
				{
					std::lock_guard<std::mutex> lock(_mutex);
					_imageResources.emplace(filePath, std::make_pair(std::move(stagingBuffer), std::move(resource.dimensions)));
				}
				ImageLoader::deallocateResources(resource);
			}
		);
	}
	else {
		ImageResource resource = ImageLoader::load2DImage(filePath);
		_imageResources.emplace(filePath, std::make_pair(StagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size)), resource.dimensions));
		ImageLoader::deallocateResources(resource);
	}
	return CacheCode::NOT_CACHED;
}

CacheCode AssetManager::loadImageCubemap(std::string_view filePath) {
	ImageResource resource = ImageLoader::loadCubemapImage(filePath);
	_imageResources.emplace(filePath, std::make_pair(StagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size)), resource.dimensions));
	ImageLoader::deallocateResources(resource);
	return CacheCode::NOT_CACHED;
}

const std::pair<StagingBuffer, ImageDimensions>& AssetManager::getImageData(const std::string& filePath) const {
	auto it = _imageResources.find(filePath);
	if (it != _imageResources.cend())
		return it->second;
	throw std::runtime_error("Image data not found");
}

CacheCode AssetManager::deleteImage(std::string_view filePath) {
	return _imageResources.erase(std::string{ filePath }) ? CacheCode::CACHED : CacheCode::NOT_CACHED;
}
