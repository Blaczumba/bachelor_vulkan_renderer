#include "asset_manager.h"

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(MemoryAllocator& memoryAllocator) : _memoryAllocator(memoryAllocator) {}

std::shared_future<std::unique_ptr<VertexData>> AssetManager::loadVertexDataAsync(const std::string& identifier, std::function<ImageResource()>&& loadingFunction) {
    auto it = _awaitingVertexDataResources.find(identifier);
    if (it != _awaitingVertexDataResources.end()) {
        return it->second;
    }

    //auto future = std::async(std::launch::async, ([this, filePath, loadingFunction = std::move(loadingFunction)]() {
    //    ImageResource resource = loadingFunction(filePath);
    //    StagingBuffer stagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size));
    //    ImageLoader::deallocateResources(resource);
    //    return std::make_unique<ImageData>(std::move(stagingBuffer), std::move(resource.dimensions));
    //})).share();

    //_awaitingVertexDataResources.emplace(filePath, future);
    //return future;
}

std::shared_future<std::unique_ptr<ImageData>> AssetManager::loadImageAsync(const std::string& filePath, std::function<ImageResource(std::string_view)>&& loadingFunction) {
    auto it = _awaitingImageResources.find(filePath);
    if (it != _awaitingImageResources.end()) {
        return it->second;
    }

    auto future = std::async(std::launch::async, ([this, filePath, loadingFunction = std::move(loadingFunction)]() {
        ImageResource resource = loadingFunction(filePath);
        StagingBuffer stagingBuffer(_memoryAllocator, std::span(static_cast<uint8_t*>(resource.data), resource.size));
        ImageLoader::deallocateResources(resource);
        return std::make_unique<ImageData>(std::move(stagingBuffer), std::move(resource.dimensions));
    })).share();

    _awaitingImageResources.emplace(filePath, future);
    return future;
}

std::shared_future<std::unique_ptr<ImageData>> AssetManager::loadImage2DAsync(const std::string& filePath) {
	return loadImageAsync(filePath, ImageLoader::load2DImage);
}

std::shared_future<std::unique_ptr<ImageData>> AssetManager::loadImageCubemapAsync(const std::string& filePath) {
	return loadImageAsync(filePath, ImageLoader::loadCubemapImage);
}

const ImageData& AssetManager::getImageData(const std::string& filePath) {
    auto imageIt = _imageResources.find(filePath);
    if (imageIt != _imageResources.cend()) {
        return imageIt->second;
    }
	auto it = _awaitingImageResources.find(filePath);
    if (it != _awaitingImageResources.cend()) {
        auto ptr = _imageResources.emplace(filePath, std::move(*it->second.get()));
        _awaitingImageResources.erase(it);
		return ptr.first->second;
    }
	throw std::runtime_error("Image data not found");
}

void AssetManager::deleteImage(std::string_view filePath) {

	return;
}
