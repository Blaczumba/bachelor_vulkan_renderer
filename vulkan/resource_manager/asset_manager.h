#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "common/file/file_loader.h"
#include "common/model_loader/image_loader/image_loader.h"
#include "common/util/asset_manager.h"
#include "common/util/buffer_manip.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/util/index_buffer_util.h"

class AssetManager : public common::AssetManager<AssetManager> {
public:
  AssetManager() = default;

  AssetManager(const LogicalDevice& logicalDevice, const std::shared_ptr<FileLoader>& fileLoader,
               std::launch launchPolicy = std::launch::async);

  AssetManager& operator=(AssetManager&& assetManager) noexcept;

  ~AssetManager() = default;

  struct ImageData {
    Buffer stagingBuffer;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    uint32_t layerCount;
    std::vector<ImageSubresource> copyRegions;
  };

  struct VertexData {
    std::unordered_map<std::string, Buffer> buffers;
    Buffer indexBuffer;
    VkIndexType indexType;
  };

  void loadImageAsync(const std::string& filePath);

  template <typename Model, typename... Type>
  void loadVertexDataInterleavingAsync(
      std::shared_ptr<Model>& modelPtr, const std::string& name, std::span<const std::byte> indices,
      uint8_t indexSize, std::span<const std::pair<std::string, std::string>> orders,
      std::span<const Type>... attributes);

  template <typename VertexType, typename Model>
  void loadVertexDataAsync(
      std::shared_ptr<Model>& modelPtr, const std::string& filePath,
      std::span<const std::byte> indices, uint8_t indexSize, std::span<const VertexType> data);

  const ImageData& getImageData(const std::string& filePath);

  const VertexData& getVertexData(const std::string& filePath);

private:
  void loadImageAsync(const std::string& filePath,
                      std::function<ImageResource(std::span<const std::byte>)>&& loadingFunction);

  std::launch _launchPolicy;

  const LogicalDevice* _logicalDevice = nullptr;

  std::shared_ptr<FileLoader> _fileLoader;

  std::unordered_map<std::string, VertexData> _vertexDataResources;
  std::unordered_map<std::string, std::future<VertexData>> _awaitingVertexDataResources;

  std::unordered_map<std::string, ImageData> _imageResources;
  std::unordered_map<std::string, std::future<ImageData>> _awaitingImageResources;
};

template <typename Model, typename... Type>
void AssetManager::loadVertexDataInterleavingAsync(
    std::shared_ptr<Model>& modelPtr, const std::string& name, std::span<const std::byte> indices,
    uint8_t indexSize, std::span<const std::pair<std::string, std::string>> orders,
    std::span<const Type>... attributes) {
  if (_awaitingVertexDataResources.contains(name)) {
    return;
  }

  std::future<VertexData> future = std::async(
      _launchPolicy, [this, modelPtr, indices, indexSize, orders, attributes...]() -> VertexData {
        VertexData vertexData;
        const AttributeDescription descs[] = {
          AttributeDescription{(void*)attributes.data(), sizeof(Type), attributes.size()}
          ...
        };

        std::vector<BufferDescription> bufferDescriptions = analyzeConfig(orders, descs);

        for (BufferDescription& description : bufferDescriptions) {
          Buffer vertexBuffer = Buffer::createStagingBuffer(*_logicalDevice, description.totalSize);
          vertexBuffer.copyDataInterleaving(description.attributes);
          vertexData.buffers.emplace(std::move(description.name), std::move(vertexBuffer));
        }

        const size_t shrunkIndexSize = getShrunkIndexSize(indices, indexSize);
        vertexData.indexBuffer = Buffer::createStagingBuffer(
            *_logicalDevice, indices.size() / indexSize * shrunkIndexSize);

        vertexData.indexBuffer.copyAndShrinkData(indices, shrunkIndexSize, indexSize);

        vertexData.indexType = getIndexType(shrunkIndexSize);

        return vertexData;
      });
  _awaitingVertexDataResources.emplace(name, std::move(future));
}

template <typename Type, typename Model>
void AssetManager::loadVertexDataAsync(
    std::shared_ptr<Model>& modelPtr, const std::string& name, std::span<const std::byte> indices,
    uint8_t indexSize, std::span<const Type> vertices) {
  if (_awaitingVertexDataResources.contains(name)) {
    return;
  }

  std::future<VertexData> future = std::async(
      _launchPolicy,
      [this, modelPtr, indices, indexSize,
       vertices]() -> VertexData {  // TODO: boost::asio::post,
                                    // boost::asio::use_future
        Buffer vertexBuffer =
            Buffer::createStagingBuffer(*_logicalDevice, vertices.size() * sizeof(Type));
        vertexBuffer.copyData(vertices);

        Buffer indexBuffer = Buffer::createStagingBuffer(*_logicalDevice, indices.size());
        indexBuffer.copyData(indices);
        return VertexData{
          Buffer(), std::move(indexBuffer), getIndexType(indexSize), std::move(vertexBuffer)};
      });
  _awaitingVertexDataResources.emplace(name, std::move(future));
}
