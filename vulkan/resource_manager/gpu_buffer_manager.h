#pragma once

#include <common/util/resource_handles.h>
#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"

class GpuBufferManager {
  struct BufferResource {
    Buffer buffer;
    size_t refCount = 0;
  };

  using GpuBufferMap = lib::SparseMap<BufferResource, MAX_GPU_BUFFERS>;

  struct TextureResource {
    Texture texture;
    size_t refCount = 0;
  };

  using GpuTextureMap = lib::SparseMap<TextureResource, MAX_GPU_TEXTURES>;

  GpuBufferManager() noexcept = default;

public:
  ~GpuBufferManager() = default;

  static std::unique_ptr<GpuBufferManager> create();

  void increaseRefCount(GpuBufferHandle index);
 
  void decreaseRefCount(GpuBufferHandle index);

  void increaseRefCount(GpuTextureHandle index);

  void decreaseRefCount(GpuTextureHandle index);

  enum class BufferType : uint8_t {
    VERTEX,
    INDEX
  };

  GpuBufferHandle uploadBuffer(
      VkCommandBuffer commandBuffer, const Buffer& stagingBuffer, BufferType bufferType);

  GpuBufferHandle transferBuffer(Buffer&& stagingBuffer);

  const Buffer& getBuffer(GpuBufferHandle index) const;

  bool removeBuffer(GpuBufferHandle index);

  GpuTextureHandle transferTexture(Texture&& stagingBuffer);

  const Texture& getTexture(GpuTextureHandle index) const;

  bool removeTexture(GpuTextureHandle index);

private:
  GpuBufferMap _bufferMap;
  std::vector<GpuBufferHandle> _freeBufferIndices;

  GpuTextureMap _textureMap;
  std::vector<GpuTextureHandle> _freeTextureIndices;
};
