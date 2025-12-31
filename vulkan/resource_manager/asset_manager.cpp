#include "asset_manager.h"

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(const LogicalDevice& logicalDevice,
                           const std::shared_ptr<FileLoader>& fileLoader, std::launch launchPolicy)
  : _logicalDevice(&logicalDevice), _fileLoader(fileLoader), _launchPolicy(launchPolicy),
    _freeVertexDataIndices(MAX_VERTEX_DATA_RESOURCES) {
  std::iota(_freeVertexDataIndices.rbegin(), _freeVertexDataIndices.rend(), 0);
}

AssetManager& AssetManager::operator=(AssetManager&& assetManager) noexcept {
  if (this == &assetManager) {
    return *this;
  }

  _logicalDevice = std::exchange(assetManager._logicalDevice, nullptr);
  _fileLoader = std::move(assetManager._fileLoader);
  // _vertexDataResources = std::move(assetManager._vertexDataResources); // TODO:
  _awaitingVertexDataResources = std::move(assetManager._awaitingVertexDataResources);
  _imageResources = std::move(assetManager._imageResources);
  _awaitingImageResources = std::move(assetManager._awaitingImageResources);
  _freeVertexDataIndices = std::move(assetManager._freeVertexDataIndices);
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

const VertexData& AssetManager::getVertexData(VertexResourceMapIndex index) {
  if (_vertexDataResources.exists(index)) [[likely]] {
    return _vertexDataResources.getValue(index);
  }

  auto it = _awaitingVertexDataResources.find(index);
  if (it == _awaitingVertexDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", index));
  }

  const VertexData& ptr = _vertexDataResources.insertUnsafe(index, it->second.get());
  _awaitingVertexDataResources.erase(it);
  return ptr;
}
