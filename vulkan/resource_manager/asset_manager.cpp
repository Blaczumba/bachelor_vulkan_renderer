#include "asset_manager.h"

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy)
  : _logicalDevice(logicalDevice), _fileLoader(fileLoader), _launchPolicy(launchPolicy),
    _freeImageDataIndices(MAX_IMAGE_DATA_RESOURCES),
    _freeVertexDataIndices(MAX_VERTEX_DATA_RESOURCES) {
  std::iota(_freeImageDataIndices.rbegin(), _freeImageDataIndices.rend(), 0);
  std::iota(_freeVertexDataIndices.rbegin(), _freeVertexDataIndices.rend(), 0);
}

std::unique_ptr<AssetManager> AssetManager::create(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy) {
  return std::unique_ptr<AssetManager>(new AssetManager(logicalDevice, fileLoader, launchPolicy));
}

size_t AssetManager::loadImageAsync(
    const std::string& filePath,
    std::function<ImageResource(std::span<const std::byte>)> loadingFunction) {
  const ImageResourceMapIndex index = _freeImageDataIndices.back();
  _freeImageDataIndices.pop_back();
  _awaitingImageDataResources.emplace(
      index,
      std::async(
          _launchPolicy,
          [this, filePath, loadingFunction = std::move(loadingFunction)]() mutable -> ImageData {
            ImageResource resource = loadingFunction(_fileLoader.loadFileToBuffer(filePath));

            Buffer stagingBuffer = Buffer::createStagingBuffer(_logicalDevice, resource.size);
            stagingBuffer.copyData(
                std::span(static_cast<const std::byte*>(resource.data), resource.size));
            ImageLoader::deallocateResources(resource);
            return ImageData(
                std::move(stagingBuffer), resource.width, resource.height, resource.mipLevels,
                resource.layerCount, std::move(resource.subresources));
          }));

  return index;
}

size_t AssetManager::loadImageAsync(const std::string& filePath) {
  if (filePath.ends_with(".ktx") || filePath.ends_with(".ktx2")) {
    return loadImageAsync(filePath, ImageLoader::loadImageKtx);
  } else {
    return loadImageAsync(filePath, ImageLoader::loadImageStbi);
  }
}

const ImageData& AssetManager::getImageData(size_t index) {
  if (_imageDataResources.exists(index)) [[likely]] {
    return _imageDataResources.getValue(index);
  }

  auto it = _awaitingImageDataResources.find(index);
  if (it == _awaitingImageDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", index));
  }

  const ImageData& ptr = _imageDataResources.insertUnsafe(index, it->second.get());
  _awaitingImageDataResources.erase(it);
  return ptr;
}

const VertexData& AssetManager::getVertexData(size_t index) {
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
