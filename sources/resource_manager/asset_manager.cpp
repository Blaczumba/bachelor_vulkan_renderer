#include "asset_manager.h"

AssetManager::AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool) : _memoryAllocator(memoryAllocator), _threadPool(threadPool), _index(0) {}

std::shared_future<std::unique_ptr<std::pair<StagingBuffer, ImageDimensions>>> AssetManager::loadImageAsync(const std::string& filePath, std::function<ImageResource(std::string_view)>&& loadingFunction) {
	auto promise = std::make_shared<std::promise<std::unique_ptr<std::pair<StagingBuffer, ImageDimensions>>>>();
	std::shared_future<std::unique_ptr<std::pair<StagingBuffer, ImageDimensions>>> future = promise->get_future().share();
	{
		std::unique_lock<std::mutex> lock(_awaitingImageMutex);
		auto it = _awaitingImageResources.find(filePath);
		if (it != _awaitingImageResources.cend()) {
			return it->second;
		}
		_awaitingImageResources.emplace(filePath, future);
	}
	_threadPool->getThread((++_index) % _threadPool->getNumThreads()).addJob([this, filePath, promise = std::move(promise), loadingFunction = std::move(loadingFunction)]() {
			ImageResource resource = loadingFunction(filePath);
			StagingBuffer stagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size));
			{
				std::lock_guard<std::mutex> lock(_awaitingImageMutex);
				promise->set_value(std::make_unique<std::pair<StagingBuffer, ImageDimensions>>(std::move(stagingBuffer), std::move(resource.dimensions)));
			}
			ImageLoader::deallocateResources(resource);
		}
	);
	return future;
}

std::shared_future<std::unique_ptr<std::pair<StagingBuffer, ImageDimensions>>> AssetManager::loadImage2DAsync(const std::string& filePath) {
	return loadImageAsync(filePath, ImageLoader::load2DImage);
}

std::shared_future<std::unique_ptr<std::pair<StagingBuffer, ImageDimensions>>> AssetManager::loadImageCubemapAsync(const std::string& filePath) {
	return loadImageAsync(filePath, ImageLoader::loadCubemapImage);
}

const std::pair<StagingBuffer, ImageDimensions>& AssetManager::getImageData(const std::string& filePath) const {
	auto it = _awaitingImageResources.find(filePath);
	if (it != _awaitingImageResources.cend())
		return *it->second.get();
	throw std::runtime_error("Image data not found");
}

void AssetManager::deleteImage(std::string_view filePath) {

	return;
}
