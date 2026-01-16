#pragma once

#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/memory_objects/buffer.h"

class GpuBufferManager {
  static constexpr size_t MAX_BUFFERS = 256;
  using GpuBufferMap = lib::SparseMap<Buffer, MAX_BUFFERS>;

  GpuBufferManager() noexcept = default;

public:
  ~GpuBufferManager() = default;

  static std::unique_ptr<GpuBufferManager> create();

  DEFINE_STRONG_INT(GpuBufferMapIndex, GpuBufferMap::IndexType);

  enum class BufferType : uint8_t {
	VERTEX,
	INDEX
  };

  GpuBufferMapIndex uploadBuffer(VkCommandBuffer commandBuffer, Buffer& stagingBuffer, BufferType bufferType);

  GpuBufferMapIndex transferBuffer(Buffer&& stagingBuffer);

  bool removeBuffer(GpuBufferMapIndex index);

private:
  GpuBufferMap _bufferMap;
  std::vector<GpuBufferMapIndex> _freeBufferIndices;
};
