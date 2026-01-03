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
#include "lib/association_list/association_list.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/util/index_buffer_util.h"

class AssetManager : public common::AssetManager<AssetManager> {
  AssetManager(
      const LogicalDevice& logicalDevice, const FileLoader& fileLoader, std::launch launchPolicy);

public:
  static std::unique_ptr<AssetManager> create(
      const LogicalDevice& logicalDevice, const FileLoader& fileLoader,
      std::launch launchPolicy = std::launch::async);

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
    lib::DynamicAssociationList<std::string, Buffer> buffers;
    Buffer indexBuffer;
    VkIndexType indexType;
  };

  size_t loadImageAsync(const std::string& filePath);

  template <typename Model, typename... Type>
  size_t loadVertexDataInterleavingAsync(
      std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::span<const std::pair<std::string, std::string>> orders,
      std::span<const Type>... attributes);

  const ImageData& getImageData(size_t index);

  const VertexData& getVertexData(size_t index);

private:
  size_t loadImageAsync(
      const std::string& filePath,
      std::function<ImageResource(std::span<const std::byte>)> loadingFunction);

  std::launch _launchPolicy;

  const LogicalDevice& _logicalDevice;
  const FileLoader& _fileLoader;

  static constexpr size_t MAX_IMAGE_DATA_RESOURCES = 256;
  using ImageResourceMap = lib::SparseMap<ImageData, MAX_IMAGE_DATA_RESOURCES>;
  using ImageResourceMapIndex = typename ImageResourceMap::IndexType;

  std::vector<ImageResourceMapIndex> _freeImageDataIndices;  // TODO: Change to inplace vector.
  std::unordered_map<ImageResourceMapIndex, std::future<ImageData>>
      _awaitingImageDataResources;  // TODO: Change to flat unordered map.
  ImageResourceMap _imageDataResources;

  static constexpr size_t MAX_VERTEX_DATA_RESOURCES = 256;
  using VertexResourceMap = lib::SparseMap<VertexData, MAX_VERTEX_DATA_RESOURCES>;
  using VertexResourceMapIndex = typename VertexResourceMap::IndexType;

  std::vector<VertexResourceMapIndex> _freeVertexDataIndices;  // TODO: Change to inplace vector.
  std::unordered_map<VertexResourceMapIndex, std::future<VertexData>>
      _awaitingVertexDataResources;  // TODO: Change to flat unordered map.
  VertexResourceMap _vertexDataResources;
};

template <typename Model, typename... Type>
size_t AssetManager::loadVertexDataInterleavingAsync(
    std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
    std::span<const std::pair<std::string, std::string>> orders,
    std::span<const Type>... attributes) {
  const VertexResourceMapIndex index = _freeVertexDataIndices.back();
  _freeVertexDataIndices.pop_back();
  _awaitingVertexDataResources.emplace(
      index,
      std::async(
          _launchPolicy,
          [this, modelPtr, indices, indexSize, orders, attributes...]() -> VertexData {
            VertexData vertexData;
            const AttributeDescription descs[] = {
              AttributeDescription{(void*)attributes.data(), sizeof(Type), attributes.size()}
              ...
            };

            std::vector<BufferDescription> bufferDescriptions = analyzeConfig(orders, descs);

            for (BufferDescription& description : bufferDescriptions) {
              Buffer vertexBuffer =
                  Buffer::createStagingBuffer(_logicalDevice, description.totalSize);
              vertexBuffer.copyDataInterleaving(description.attributes);
              vertexData.buffers.insert({std::move(description.name), std::move(vertexBuffer)});
            }

            const size_t shrunkIndexSize = getShrunkIndexSize(indices, indexSize);
            vertexData.indexBuffer = Buffer::createStagingBuffer(
                _logicalDevice, indices.size() / indexSize * shrunkIndexSize);

            vertexData.indexBuffer.copyAndShrinkData(indices, shrunkIndexSize, indexSize);

            vertexData.indexType = getIndexType(shrunkIndexSize);

            return vertexData;
          }));

  return index;
}
