#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <span>
#include <string>
#include <unordered_map>

#include "common/file/file_loader.h"
#include "common/model_loader/image_loader/image_loader.h"
#include "common/util/asset_manager.h"
#include "common/util/buffer_manip.h"
#include "lib/association_list/association_list.h"
#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
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
    lib::Buffer<VkBufferImageCopy> copyRegions;
  };

  struct VertexData {
    lib::DynamicAssociationList<std::string, Buffer> buffers;
    Buffer indexBuffer;
    VkIndexType indexType;
  };

private:
  static constexpr size_t MAX_IMAGE_DATA_RESOURCES = 256;
  using ImageResourceMap = lib::SparseMap<ImageData, MAX_IMAGE_DATA_RESOURCES>;

  static constexpr size_t MAX_VERTEX_DATA_RESOURCES = 256;
  using VertexResourceMap = lib::SparseMap<VertexData, MAX_VERTEX_DATA_RESOURCES>;

public:
  DEFINE_STRONG_INT(ImageResourceMapIndex, typename ImageResourceMap::IndexType);
  DEFINE_STRONG_INT(VertexResourceMapIndex, typename VertexResourceMap::IndexType);

  ImageResourceMapIndex loadImageAsync(const std::string& filePath);

  template <typename Model, typename... Type>
  VertexResourceMapIndex loadVertexDataInterleavingAsync(
      std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::span<const std::pair<std::string, std::string>> orders,
      std::span<const Type>... attributes);

  const ImageData& getImageData(ImageResourceMapIndex index);

  const VertexData& getVertexData(VertexResourceMapIndex index);

private:
// TODO: Change after std::move_only_function becomes a standard.
#ifdef ANDROID
  using ImageJob = std::function<ImageResource(std::span<const std::byte>)>;
#else
  using ImageJob = std::move_only_function<ImageResource(std::span<const std::byte>)>;
#endif  // ANDROID

  ImageResourceMapIndex loadImageAsync(const std::string& filePath, ImageJob loadingFunction);

  std::launch _launchPolicy;

  const LogicalDevice& _logicalDevice;
  const FileLoader& _fileLoader;

  std::vector<ImageResourceMapIndex> _freeImageDataIndices;  // TODO: Change to inplace vector.
  std::unordered_map<ImageResourceMapIndex, std::future<ImageData>>
      _awaitingImageDataResources;  // TODO: Change to flat unordered map.
  ImageResourceMap _imageDataResources;

  std::vector<VertexResourceMapIndex> _freeVertexDataIndices;  // TODO: Change to inplace vector.
  std::unordered_map<VertexResourceMapIndex, std::future<VertexData>>
      _awaitingVertexDataResources;  // TODO: Change to flat unordered map.
  VertexResourceMap _vertexDataResources;
};

template <typename Model, typename... Type>
AssetManager::VertexResourceMapIndex AssetManager::loadVertexDataInterleavingAsync(
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
