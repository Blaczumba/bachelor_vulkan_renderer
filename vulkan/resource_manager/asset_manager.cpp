#include "asset_manager.h"

#include <algorithm>
#include <format>
#include <functional>
#include <future>
#include <memory>
#include <numeric>
#include <span>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/util/index_buffer_util.h"

using ImageData = AssetManager::ImageData;
using VertexData = AssetManager::VertexData;

AssetManager::AssetManager(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy)
  : _logicalDevice(logicalDevice), _fileLoader(fileLoader), _launchPolicy(std::launch::deferred),
    _freeImageDataIndices(MAX_STAGING_IMAGE_DATA_RESOURCES),
    _freeVertexDataIndices(MAX_STAGING_VERTEX_DATA_RESOURCES) {
  std::iota(_freeImageDataIndices.rbegin(), _freeImageDataIndices.rend(),
            StagingImageDataResourceHandle(0));
  std::iota(_freeVertexDataIndices.rbegin(), _freeVertexDataIndices.rend(),
            StagingVertexDataResourceHandle(0));
}

std::unique_ptr<AssetManager> AssetManager::create(
    const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy) {
  return std::unique_ptr<AssetManager>(new AssetManager(logicalDevice, fileLoader, launchPolicy));
}

namespace {

lib::Buffer<VkBufferImageCopy> translateToVkBufferImageCopy(
    std::span<const ImageSubresource> imageSubresources) {
  lib::Buffer<VkBufferImageCopy> vkSubresources(imageSubresources.size());
  std::transform(std::cbegin(imageSubresources), std::cend(imageSubresources), vkSubresources.begin(),
                 [](const ImageSubresource& subresource) {
                   return VkBufferImageCopy{
                     .bufferOffset = subresource.offset,
                     .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                          .mipLevel = subresource.mipLevel,
                                          .baseArrayLayer = subresource.baseArrayLayer,
                                          .layerCount = subresource.layerCount},
                     .imageExtent = {.width = subresource.width,
                                          .height = subresource.height,
                                          .depth = subresource.depth},
                   };
                 });
  return vkSubresources;
}

}  // namespace

StagingImageDataResourceHandle AssetManager::loadImageAsync(const std::string& filePath) {
  const StagingImageDataResourceHandle index = _freeImageDataIndices.back();
  _freeImageDataIndices.pop_back();
  _awaitingImageDataResources.emplace(
      index, std::async(_launchPolicy, [this, filePath]() -> ImageData {
        const auto [resource, dataPtr] =
            loadImage(_fileLoader.loadFileToBuffer(filePath), filePath);
        Buffer stagingBuffer = Buffer::createStagingBuffer(
            _logicalDevice, resource.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        common::copyData(stagingBuffer.getMappedMemory(), 0,
                         std::span(static_cast<const std::byte*>(resource.data), resource.size));
        return ImageData(
            std::move(stagingBuffer), resource.width, resource.height, resource.mipLevels,
            resource.layerCount, translateToVkBufferImageCopy(resource.subresources));
      }));
  return index;
}

StagingImageDataResourceHandle AssetManager::loadImageAsync(
    std::shared_ptr<void> modelPtr, std::span<const std::byte> data) {
  const StagingImageDataResourceHandle index = _freeImageDataIndices.back();
  _freeImageDataIndices.pop_back();
  _awaitingImageDataResources.emplace(
      index, std::async(_launchPolicy, [this, modelPtr = std::move(modelPtr), data]() -> ImageData {
        const auto [resource, dataPtr] = loadImage(data, "");  // TODO: refactor.
        Buffer stagingBuffer = Buffer::createStagingBuffer(
            _logicalDevice, resource.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        common::copyData(stagingBuffer.getMappedMemory(), 0,
                         std::span(static_cast<const std::byte*>(resource.data), resource.size));
        return ImageData(
            std::move(stagingBuffer), resource.width, resource.height, resource.mipLevels,
            resource.layerCount, translateToVkBufferImageCopy(resource.subresources));
      }));
  return index;
}

StagingImageDataResourceHandle AssetManager::loadImageAsync(
    std::shared_ptr<void> modelPtr, ImageResource&& imageResource) {
  const StagingImageDataResourceHandle index = _freeImageDataIndices.back();
  _freeImageDataIndices.pop_back();
  _awaitingImageDataResources.emplace(
      index,
      std::async(
          _launchPolicy,
          [this, modelPtr = std::move(modelPtr),
           imageResource = std::move(imageResource)]() -> ImageData {
            Buffer stagingBuffer = Buffer::createStagingBuffer(
                _logicalDevice, imageResource.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            common::copyData(
                stagingBuffer.getMappedMemory(), 0,
                std::span(static_cast<const std::byte*>(imageResource.data), imageResource.size), 0,
                imageResource.size);
            return ImageData(std::move(stagingBuffer), imageResource.width, imageResource.height,
                             imageResource.mipLevels, imageResource.layerCount,
                             translateToVkBufferImageCopy(imageResource.subresources));
          }));
  return index;
}

StagingVertexDataResourceHandle AssetManager::loadVertexDataInterleavingAsync(
    std::shared_ptr<void> modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
    std::vector<common::BufferDescription>&& bufferDescriptions) {
  const StagingVertexDataResourceHandle index = _freeVertexDataIndices.back();
  _freeVertexDataIndices.pop_back();
  _awaitingVertexDataResources.emplace(
      index,
      std::async(
          _launchPolicy,
          [this, modelPtr = std::move(modelPtr), indices, indexSize,
           bufferDescriptions = std::move(bufferDescriptions)]() mutable -> VertexData {
            VertexData vertexData;
            const VkPhysicalDeviceType deviceType =
                _logicalDevice.getPhysicalDevice().getPhysicalDeviceType();

            struct {
              VkBufferUsageFlags vertexBufferUsage = 0;
              VkBufferUsageFlags indexBufferUsage = 0;
            } additionalFlags;

            // For integrated graphics we create buffers properly in place so that they do not need
            // to be copied to the same memory later.
            if (deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
              additionalFlags.vertexBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
              additionalFlags.indexBufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            } else if (deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
              additionalFlags.vertexBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
              additionalFlags.indexBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }

            for (common::BufferDescription& description : bufferDescriptions) {
              Buffer vertexBuffer = Buffer::createStagingBuffer(
                  _logicalDevice, description.totalSize, additionalFlags.vertexBufferUsage);
              common::copyDataInterleaving(vertexBuffer.getMappedMemory(), description.attributes);
              vertexData.buffers.insert({std::move(description.name), std::move(vertexBuffer)});
            }

            const size_t shrunkIndexSize = getShrunkIndexSize(indices, indexSize);
            vertexData.indexBuffer = Buffer::createStagingBuffer(
                _logicalDevice, indices.size() / indexSize * shrunkIndexSize,
                additionalFlags.indexBufferUsage);
            common::copyAndShrinkIndexData(
                vertexData.indexBuffer.getMappedMemory(), indices, shrunkIndexSize, indexSize);

            vertexData.indexType = getIndexType(shrunkIndexSize);

            return vertexData;
          }));

  return index;
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
