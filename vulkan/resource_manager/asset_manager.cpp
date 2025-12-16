#include "asset_manager.h"

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(const LogicalDevice& logicalDevice,
                           const std::shared_ptr<FileLoader>& fileLoader, std::launch launchPolicy)
  : _logicalDevice(&logicalDevice), _fileLoader(fileLoader), _launchPolicy(launchPolicy) {}

AssetManager& AssetManager::operator=(AssetManager&& assetManager) noexcept {
  if (this == &assetManager) {
    return *this;
  }

  _logicalDevice = std::exchange(assetManager._logicalDevice, nullptr);
  _fileLoader = std::move(assetManager._fileLoader);
  _vertexDataResources = std::move(assetManager._vertexDataResources);
  _awaitingVertexDataResources = std::move(assetManager._awaitingVertexDataResources);
  _imageResources = std::move(assetManager._imageResources);
  _awaitingImageResources = std::move(assetManager._awaitingImageResources);
  return *this;
}

void AssetManager::loadImageAsync(
    const std::string& filePath,
    std::function<ImageResource(std::span<const std::byte>)>&& loadingFunction) {
  if (_awaitingImageResources.contains(filePath)) {
    return;
  }

  std::future<ImageData> future = std::async(
      _launchPolicy, [this, filePath, loadingFunction = std::move(loadingFunction)]() -> ImageData {
        ImageResource resource = loadingFunction(_fileLoader->loadFileToBuffer(filePath));

        Buffer stagingBuffer = Buffer::createStagingBuffer(*_logicalDevice, resource.size);
        stagingBuffer.copyData(
            std::span(static_cast<const std::byte*>(resource.data), resource.size));
        ImageLoader::deallocateResources(resource);
        return ImageData(std::move(stagingBuffer), resource.width, resource.height,
                         resource.mipLevels, resource.layerCount, std::move(resource.subresources));
      });
  _awaitingImageResources.emplace(filePath, std::move(future));
}

void AssetManager::loadImageAsync(const std::string& filePath) {
  if (filePath.ends_with(".ktx") || filePath.ends_with(".ktx2")) {
    loadImageAsync(filePath, ImageLoader::loadImageKtx);
  } else {
    loadImageAsync(filePath, ImageLoader::loadImageStbi);
  }
}

const ImageData& AssetManager::getImageData(const std::string& filePath) {
  auto imageIt = _imageResources.find(filePath);
  if (imageIt != _imageResources.cend()) {
    return imageIt->second;
  }

  auto it = _awaitingImageResources.find(filePath);
  if (it != _awaitingImageResources.cend()) {
    auto ptr = _imageResources.emplace(filePath, it->second.get());
    _awaitingImageResources.erase(it);
    return ptr.first->second;
  }

  throw EngineException(std::format("Failed to find {} in AssetManager.", filePath));
}

const VertexData& AssetManager::getVertexData(const std::string& filePath) {
  auto vertexIt = _vertexDataResources.find(filePath);
  if (vertexIt != _vertexDataResources.cend()) {
    return vertexIt->second;
  }

  auto it = _awaitingVertexDataResources.find(filePath);
  if (it != _awaitingVertexDataResources.cend()) {
    auto ptr = _vertexDataResources.emplace(filePath, it->second.get());
    _awaitingVertexDataResources.erase(it);
    return ptr.first->second;
  }

  throw EngineException(std::format("Failed to find {} in AssetManager.", filePath));
}
