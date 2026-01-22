#pragma once

#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"

class GpuBufferManager {
  static constexpr size_t MAX_BUFFERS = 1024;

  struct BufferResource {
    Buffer buffer;
    size_t refCount = 0;
  };

  using GpuBufferMap = lib::SparseMap<BufferResource, MAX_BUFFERS>;

  static constexpr size_t MAX_TEXTURES = 1024;

  struct TextureResource {
    Texture texture;
    size_t refCount = 0;
  };

  using GpuTextureMap = lib::SparseMap<TextureResource, MAX_TEXTURES>;

  GpuBufferManager() noexcept = default;

public:
  ~GpuBufferManager() = default;

  static std::unique_ptr<GpuBufferManager> create();

  DEFINE_STRONG_INT(GpuBufferMapIndex, GpuBufferMap::IndexType);
  DEFINE_STRONG_INT(GpuTextureMapIndex, GpuTextureMap::IndexType);

  void increaseRefCount(GpuBufferMapIndex index);

  void decreaseRefCount(GpuBufferMapIndex index);

  void increaseRefCount(GpuTextureMapIndex index);

  void decreaseRefCount(GpuTextureMapIndex index);

  enum class BufferType : uint8_t {
    VERTEX,
    INDEX
  };

  GpuBufferMapIndex uploadBuffer(
      VkCommandBuffer commandBuffer, const Buffer& stagingBuffer, BufferType bufferType);

  GpuBufferMapIndex transferBuffer(Buffer&& stagingBuffer);

  const Buffer& getBuffer(GpuBufferMapIndex index) const;

  bool removeBuffer(GpuBufferMapIndex index);

  GpuTextureMapIndex transferTexture(Texture&& stagingBuffer);

  const Texture& getTexture(GpuTextureMapIndex index) const;

  bool removeTexture(GpuTextureMapIndex index);

private:
  GpuBufferMap _bufferMap;
  std::vector<GpuBufferMapIndex> _freeBufferIndices;

  GpuTextureMap _textureMap;
  std::vector<GpuTextureMapIndex> _freeTextureIndices;
};
