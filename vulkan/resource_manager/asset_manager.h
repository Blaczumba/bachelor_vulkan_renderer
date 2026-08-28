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

#include "vulkan/resource_manager/buffer_manager.h"
#include "lib/types/memory.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"
#include "vulkan/wrapper/util/check.h"
#include <array>
#include <future>

class AssetManager : public common::AssetManager {
  AssetManager(const LogicalDevice& logicalDevice, std::launch launchPolicy);

public:
  static std::unique_ptr<AssetManager> create(
      const LogicalDevice& logicalDevice, std::launch launchPolicy = std::launch::async);

  ~AssetManager() = default;

  struct ImageData {
    std::tuple<Buffer, BufferMetadata> stagingBuffer;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    uint32_t layerCount;
    lib::Buffer<VkBufferImageCopy> copyRegions;
  };

  struct VertexData {
    lib::DynamicAssociationList<std::string, std::tuple<Buffer, BufferMetadata>> buffers;
    std::tuple<Buffer, BufferMetadata> indexBuffer;
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

namespace {

struct VirtualBlockInitializer {
  const size_t size;

  VirtualBlock operator()(VmaAllocator allocator) {
    const VmaVirtualBlockCreateInfo createInfo{
      .size = size, .flags = VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT};
    VmaVirtualBlock virtualBlock;
    CHECK_VKCMD(vmaCreateVirtualBlock(&createInfo, &virtualBlock),
                "Failed to allocate Virtual Block for a staging buffer - VMA.");
    return virtualBlock;
  }

  VirtualBlock operator()(auto&&) {
    return {};
  }
};

struct VirtualAllocator {
  const size_t size;
  const size_t alignment;

  std::optional<std::tuple<VirtualAllocation, size_t>> operator()(VmaVirtualBlock virtualBlock) {
    VmaVirtualAllocationCreateInfo createInfo {
        .size = size,
        .alignment = alignment
    };

    VmaVirtualAllocation allocation;
    VkDeviceSize assignedOffset;

    if (vmaVirtualAllocate(virtualBlock, &createInfo, &allocation, &assignedOffset)
        != VK_SUCCESS) {
      return std::nullopt;
    }
    return std::make_tuple(allocation, assignedOffset);
  }

  std::optional<std::tuple<VirtualAllocation, size_t>> operator()(auto&&) {
    return std::nullopt;
  }
};

}

class NewAssetManager : public common::AssetManager {
  struct ThreadData {
    std::thread thread;
    Ref<Buffer> stagingBuffer;
    VirtualBlock virtualBlock;
    std::mutex _blockMutex;
    bool stop = false;
  };

  NewAssetManager(const LogicalDevice& logicalDevice, BufferManager& bufferManager, uint8_t threadCount, size_t size = 2 * lib::GiB)
    : _threads(threadCount), _alignment(logicalDevice.getPhysicalDevice().getStagingAlignment()) {
    for (uint8_t i = 0; i < _threads.size(); i++) {
      const size_t requestedSize = size / threadCount;
      auto [buffer, metadata] = BufferBuilder()
              .withUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
              .withSize(requestedSize)
              .buildStagingBufferWithMetadata(logicalDevice);
      _threads[i].stagingBuffer = bufferManager.storeBuffer(std::move(buffer), metadata);
      _threads[i].virtualBlock = std::visit(
          VirtualBlockInitializer{requestedSize}, logicalDevice.getMemoryAllocator());
      _threads[i].thread = std::thread(&NewAssetManager::doWork, this, i);
    }
  }

  void doWork(uint8_t threadIndex) {
    ThreadData& thisThread = _threads[threadIndex];
    std::function<void(ThreadData&)> task;
    while (true) {
        {
          std::unique_lock lock(_mutex);
          _conditionVariable.wait(lock, [this] {
            return !_tasks.empty() || _stop;
          });

          if (_stop) {
            return;
          }

          task = std::move(_tasks.front());
          _tasks.pop();
        }
        task(thisThread);
    }
  }

public:
  static std::unique_ptr<AssetManager> create(const LogicalDevice& logicalDevice);

   struct ImageData {
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    uint32_t layerCount;
    lib::Buffer<VkBufferImageCopy> copyRegions;
    std::atomic<bool> ready;

  private:
    Ref<Buffer> bufferRef;

  };

  StagingImageDataResourceHandle loadImageAsync(
      std::function<std::tuple<ImageResource, OwnedImageData>(void)>&& imageFunction) override {
      auto promise = std::make_shared<std::promise<int>>();
      [promise, imageFunction = std::move(imageFunction)](ThreadData& threadData) {
        const auto [resource, dataPtr] = imageFunction();
        ImageData imageData = {
          .stagingBuffer = BufferBuilder()
                               .withSize(resource.size)
                               .withUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                               .buildStagingBufferWithMetadata(_logicalDevice),
          .width = resource.width,
          .height = resource.height,
          .mipLevels = resource.mipLevels,
          .layerCount = resource.layerCount,
          .copyRegions = translateToVkBufferImageCopy(resource.subresources),
        };
        common::copyData(std::get<BufferMetadata>(imageData.stagingBuffer).getMappedMemoryAsSpan(),
                         0, std::span(static_cast<const std::byte*>(resource.data), resource.size));

      };
  }

private:
  lib::Buffer<ThreadData> _threads;
  const size_t _alignment;

  std::mutex _mutex;
  std::condition_variable _conditionVariable;
  std::queue<std::function<void(ThreadData&)>> _tasks;
  bool _stop = false;
};
