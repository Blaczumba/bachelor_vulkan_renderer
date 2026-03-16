#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <span>
#include <string>
#include <unordered_map>

#include "common/buffer/buffer.h"
#include "common/file/file_loader.h"
#include "common/model_loader/image_loader/image_loader.h"
#include "common/util/asset_manager.h"
#include "common/util/buffer_manip.h"
#include "common/util/resource_handles.h"
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
  using ImageResourceMap = lib::SparseMap<ImageData, MAX_STAGING_IMAGE_DATA_RESOURCES>;
  using VertexResourceMap = lib::SparseMap<VertexData, MAX_STAGING_VERTEX_DATA_RESOURCES>;

public:
  StagingImageDataResourceHandle loadImageAsync(const std::string& filePath);

  template <typename Model, typename... Type>
  StagingVertexDataResourceHandle loadVertexDataInterleavingAsync(
      std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::span<const std::pair<std::string, std::string>> orders,
      std::span<const Type>... attributes);

  const ImageData& getImageData(StagingImageDataResourceHandle index);

  ImageData releaseImageData(StagingImageDataResourceHandle index);

  const VertexData& getVertexData(StagingVertexDataResourceHandle index);

  VertexData releaseVertexData(StagingVertexDataResourceHandle index);

private:
  std::launch _launchPolicy;

  const LogicalDevice& _logicalDevice;
  const FileLoader& _fileLoader;

  std::vector<StagingImageDataResourceHandle> _freeImageDataIndices;
  std::unordered_map<StagingImageDataResourceHandle, std::future<ImageData>>
      _awaitingImageDataResources;  // TODO: Change to flat unordered map.
  ImageResourceMap _imageDataResources;

  std::vector<StagingVertexDataResourceHandle> _freeVertexDataIndices;
  std::unordered_map<StagingVertexDataResourceHandle, std::future<VertexData>>
      _awaitingVertexDataResources;  // TODO: Change to flat unordered map.
  VertexResourceMap _vertexDataResources;
};

template <typename Model, typename... Type>
StagingVertexDataResourceHandle AssetManager::loadVertexDataInterleavingAsync(
    std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
    std::span<const std::pair<std::string, std::string>> orders,
    std::span<const Type>... attributes) {
  const StagingVertexDataResourceHandle index = _freeVertexDataIndices.back();
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

            for (BufferDescription& description : bufferDescriptions) {
              Buffer vertexBuffer = Buffer::createStagingBuffer(
                  _logicalDevice, description.totalSize, additionalFlags.vertexBufferUsage);
              vertexBuffer.copyDataInterleaving(description.attributes);
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
