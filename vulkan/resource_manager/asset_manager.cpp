#include "asset_manager.h"

#include <numeric>

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy)
  : _logicalDevice(logicalDevice), _fileLoader(fileLoader), _launchPolicy(launchPolicy),
    _freeImageDataIndices(MAX_STAGING_IMAGE_DATA_RESOURCES),
    _freeVertexDataIndices(MAX_STAGING_VERTEX_DATA_RESOURCES) {
  std::iota(_freeImageDataIndices.rbegin(), _freeImageDataIndices.rend(), StagingImageDataResourceHandle(0));
  std::iota(
      _freeVertexDataIndices.rbegin(), _freeVertexDataIndices.rend(), StagingVertexDataResourceHandle(0));
}

std::unique_ptr<AssetManager> AssetManager::create(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy) {
  return std::unique_ptr<AssetManager>(new AssetManager(logicalDevice, fileLoader, launchPolicy));
}

StagingImageDataResourceHandle AssetManager::loadImageAsync(
    const std::string& filePath, ImageJob loadingFunction) {
  const StagingImageDataResourceHandle index = _freeImageDataIndices.back();
  _freeImageDataIndices.pop_back();
  _awaitingImageDataResources.emplace(
      index,
      std::async(
          _launchPolicy,
          [this, filePath, loadingFunction = std::move(loadingFunction)]() mutable -> ImageData {
            ImageResource resource = loadingFunction(_fileLoader.loadFileToBuffer(filePath));
            Buffer stagingBuffer = Buffer::createStagingBuffer(_logicalDevice, resource.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            stagingBuffer.copyData(
                std::span(static_cast<const std::byte*>(resource.data), resource.size));
            ImageLoader::deallocateResources(resource);

            lib::Buffer<VkBufferImageCopy> vkSubresources(resource.subresources.size());
            std::transform(resource.subresources.cbegin(), resource.subresources.cend(),
                           vkSubresources.begin(), [](const ImageSubresource& subresource) {
                             return VkBufferImageCopy{
                               .bufferOffset = subresource.offset,
                               .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                    .mipLevel = subresource.mipLevel,
                                                    .baseArrayLayer = subresource.baseArrayLayer,
                                                    .layerCount = subresource.layerCount},
                               .imageExtent = {.width = subresource.width,
                                                    .height = subresource.height,
                                                    .depth = subresource.depth}
                             };
                           });

            return ImageData(std::move(stagingBuffer), resource.width, resource.height,
                             resource.mipLevels, resource.layerCount, std::move(vkSubresources));
          }));

  return index;
}

StagingImageDataResourceHandle AssetManager::loadImageAsync(const std::string& filePath) {
  if (filePath.ends_with(".ktx") || filePath.ends_with(".ktx2")) {
    return loadImageAsync(filePath, ImageLoader::loadImageKtx);
  } else {
    return loadImageAsync(filePath, ImageLoader::loadImageStbi);
  }
}

const ImageData& AssetManager::getImageData(StagingImageDataResourceHandle index) {
  if (_imageDataResources.exists(*index)) [[likely]] {
    return _imageDataResources.getValue(*index);
  }

  auto it = _awaitingImageDataResources.find(index);
  if (it == _awaitingImageDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", *index));
  }

  const ImageData& data = _imageDataResources.insertUnsafe(*index, it->second.get());
  _awaitingImageDataResources.erase(it);
  return data;
}

ImageData AssetManager::releaseImageData(StagingImageDataResourceHandle index) {
  if (_imageDataResources.exists(*index)) [[likely]] {
    ImageData data = std::move(_imageDataResources.getValue(*index));
    _imageDataResources.eraseUnsafe(*index);
    return data;
  }

  auto it = _awaitingImageDataResources.find(index);
  if (it == _awaitingImageDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", *index));
  }

  ImageData data = std::move(_imageDataResources.insertUnsafe(*index, it->second.get()));
  _awaitingImageDataResources.erase(it);
  return data;
}

const VertexData& AssetManager::getVertexData(StagingVertexDataResourceHandle index) {
  if (_vertexDataResources.exists(*index)) [[likely]] {
    return _vertexDataResources.getValue(*index);
  }

  auto it = _awaitingVertexDataResources.find(index);
  if (it == _awaitingVertexDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", *index));
  }

  const VertexData& data = _vertexDataResources.insertUnsafe(*index, it->second.get());
  _awaitingVertexDataResources.erase(it);
  return data;
}

VertexData AssetManager::releaseVertexData(StagingVertexDataResourceHandle index) {
  if (_vertexDataResources.exists(*index)) [[likely]] {
    VertexData data = std::move(_vertexDataResources.getValue(*index));
    _vertexDataResources.eraseUnsafe(*index);
    return data;
  }

  auto it = _awaitingVertexDataResources.find(index);
  if (it == _awaitingVertexDataResources.cend()) [[unlikely]] {
    throw EngineException(std::format("Failed to find index {} in AssetManager.", *index));
  }

  VertexData data = std::move(_vertexDataResources.insertUnsafe(*index, it->second.get()));
  _awaitingVertexDataResources.erase(it);
  return data;
}
