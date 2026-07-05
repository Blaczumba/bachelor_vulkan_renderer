#pragma once

#include "common/util/resource_handles.h"
#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/image.h"

class GpuBufferManager {
  struct BufferResource {
    BufferWithMetadata buffer;
    size_t refCount = 0;
  };

  using GpuBufferMap = lib::SparseMap<BufferResource, MAX_GPU_BUFFERS>;

  struct ImageResource {
    Image image;
    size_t refCount = 0;
  };

  using GpuTextureMap = lib::SparseMap<ImageResource, MAX_GPU_IMAGES>;

  GpuBufferManager() noexcept = default;

public:
  ~GpuBufferManager() = default;

  static std::unique_ptr<GpuBufferManager> create();

  void increaseRefCount(GpuBufferHandle index);

  void decreaseRefCount(GpuBufferHandle index);

  void increaseRefCount(GpuImageHandle index);

  void decreaseRefCount(GpuImageHandle index);

  enum class BufferType : uint8_t {
    VERTEX,
    INDEX
  };

  GpuBufferHandle storeBuffer(VkCommandBuffer commandBuffer,
                               const BufferWithMetadata& stagingBuffer, BufferType bufferType);

  GpuBufferHandle transferBuffer(BufferWithMetadata&& stagingBuffer);

  const BufferWithMetadata& getBuffer(GpuBufferHandle index) const;

  bool removeBuffer(GpuBufferHandle index);

  GpuImageHandle transferImage(Image&& stagingBuffer);

  const Image& getImage(GpuImageHandle index) const;

  bool removeImage(GpuImageHandle index);

private:
  GpuBufferMap _bufferMap;
  std::vector<GpuBufferHandle> _freeBufferIndices;

  GpuTextureMap _imageMap;
  std::vector<GpuImageHandle> _freeImageIndices;
};
