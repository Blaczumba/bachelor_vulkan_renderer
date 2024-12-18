#include "asset_manager.h"

AssetManager::AssetManager(MemoryAllocator& memoryAllocator) : _memoryAllocator(memoryAllocator) {}

CacheCode AssetManager::loadImage2D(const std::string& filePath) {
	auto it = _imageResources.find(filePath);
	if (it == _imageResources.cend()) {
		ImageResource resource = ImageLoader::load2DImage(filePath);
		_imageResources.emplace(filePath, std::make_pair(StagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size), resource.dimensions.copyRegions), resource.dimensions));
		std::free(resource.data);
		return CacheCode::NOT_CACHED;
	}
	return CacheCode::CACHED;
}

const std::pair<StagingBuffer, ImageDimensions>& AssetManager::getImageData(const std::string& filePath) const {
	auto it = _imageResources.find(filePath);
	if (it != _imageResources.cend())
		return it->second;
}

CacheCode AssetManager::deleteImage(const std::string& filePath) {
	return _imageResources.erase(filePath) ? CacheCode::CACHED : CacheCode::NOT_CACHED;
}
