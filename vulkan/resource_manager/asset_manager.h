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

class AssetManager : public common::AssetManager {
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

  StagingImageDataResourceHandle loadImageAsync(const std::string& filePath) override;

  StagingVertexDataResourceHandle loadVertexDataInterleavingAsync(
      std::shared_ptr<void> modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::vector<common::BufferDescription>&& bufferDescriptions) override;

  const ImageData& getImageData(StagingImageDataResourceHandle index);

  ImageData releaseImageData(StagingImageDataResourceHandle index);

  const VertexData& getVertexData(StagingVertexDataResourceHandle index);

  VertexData releaseVertexData(StagingVertexDataResourceHandle index);

private:
  using ImageResourceMap = lib::SparseMap<ImageData, MAX_STAGING_IMAGE_DATA_RESOURCES>;
  using VertexResourceMap = lib::SparseMap<VertexData, MAX_STAGING_VERTEX_DATA_RESOURCES>;

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
