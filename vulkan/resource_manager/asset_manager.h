#pragma once

#include <future>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/buffer/buffer.h"
#include "common/model_loader/image_loader/types.h"
#include "common/util/asset_manager.h"
#include "common/util/buffer_manip.h"
#include "common/util/resource_handles.h"
#include "lib/association_list/association_list.h"
#include "lib/buffer/buffer.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/buffer.h"

class AssetManager : public common::AssetManager {
  AssetManager(const LogicalDevice& logicalDevice, std::launch launchPolicy);

public:
  static std::unique_ptr<AssetManager> create(
      const LogicalDevice& logicalDevice, std::launch launchPolicy = std::launch::async);

  ~AssetManager() = default;

  struct ImageData {
    BufferWithMetadata stagingBuffer;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    uint32_t layerCount;
    lib::Buffer<VkBufferImageCopy> copyRegions;
  };

  struct VertexData {
    lib::DynamicAssociationList<std::string, BufferWithMetadata> buffers;
    BufferWithMetadata indexBuffer;
    VkIndexType indexType;
  };

  StagingImageDataResourceHandle loadImageAsync(
      std::function<std::tuple<ImageResource, OwnedImageData>(void)>&& imageFunction) override;

  StagingImageDataResourceHandle loadImageAsync(
      std::shared_ptr<void> modelPtr, std::span<const std::byte> data) override;

  StagingImageDataResourceHandle loadImageAsync(
      std::shared_ptr<void> modelPtr, ImageResource&& imageResource) override;

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

  std::vector<StagingImageDataResourceHandle> _freeImageDataIndices;
  std::unordered_map<StagingImageDataResourceHandle, std::future<ImageData>>
      _awaitingImageDataResources;  // TODO: Change to flat unordered map.
  ImageResourceMap _imageDataResources;

  std::vector<StagingVertexDataResourceHandle> _freeVertexDataIndices;
  std::unordered_map<StagingVertexDataResourceHandle, std::future<VertexData>>
      _awaitingVertexDataResources;  // TODO: Change to flat unordered map.
  VertexResourceMap _vertexDataResources;
};
