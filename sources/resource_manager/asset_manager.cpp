#include "asset_manager.h"

AssetManager::AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool) : _memoryAllocator(memoryAllocator), _threadPool(threadPool), _index(0) {}

std::future<StagingBuffer*> AssetManager::loadImage2DAsync(const std::string& filePath) {
	std::shared_ptr<std::promise<StagingBuffer*>> promise = std::make_shared<std::promise<StagingBuffer*>>();
	std::future<StagingBuffer*> future = promise->get_future();
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto it = _imageResources.find(filePath);
		if (it != _imageResources.cend()) {
			promise->set_value(&it->second.first);
			return future;
		}
	}
	{
		//TODO: if we assume that filePath does not repeat then we can get rid of it.
		std::unique_lock<std::mutex> lock(_awaitingImageMutex);
		auto it = _awaitingImageResources.find(filePath);
		if (it != _awaitingImageResources.cend()) {
			promise->set_value(nullptr);
			return future;
		}
		_awaitingImageResources.emplace(filePath);
	}
	_threadPool->getThread((++_index) % _threadPool->getNumThreads()).addJob([&, filePath = filePath, promise = std::move(promise)]() {
			ImageResource resource = ImageLoader::load2DImage(filePath);
			StagingBuffer stagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size));
			{
				std::lock_guard<std::mutex> lock(_mutex);
				promise->set_value(&_imageResources.emplace(filePath, std::make_pair(std::move(stagingBuffer), std::move(resource.dimensions))).first->second.first);
			}
			{
				std::lock_guard<std::mutex> lock(_awaitingImageMutex);
				_awaitingImageResources.erase(filePath);
			}
			ImageLoader::deallocateResources(resource);
		}
	);
	return future;
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
