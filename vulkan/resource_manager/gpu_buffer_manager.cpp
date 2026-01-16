#include "gpu_buffer_manager.h"

std::unique_ptr<GpuBufferManager> GpuBufferManager::create() {
  return std::unique_ptr<GpuBufferManager>(new GpuBufferManager());
}

namespace {

template <typename T>
T getNextHandle(uint32_t elementsCount, std::vector<T>& missingHandles) {
  if (missingHandles.empty()) {
    return T(elementsCount);
  }

  T it = missingHandles.back();
  missingHandles.pop_back();
  return it;
}

}  // namespace

GpuBufferManager::GpuBufferMapIndex GpuBufferManager::uploadBuffer(
    VkCommandBuffer commandBuffer, const Buffer& stagingBuffer, BufferType bufferType) {
  const LogicalDevice& logicalDevice = stagingBuffer.getLogicalDevice();
  GpuBufferMapIndex index = getNextHandle(_bufferMap.size(), _freeBufferIndices);

  Buffer buffer;
  if (bufferType == BufferType::VERTEX) {
    buffer = Buffer::createVertexBuffer(logicalDevice, stagingBuffer.getSize());
  } else if (bufferType == BufferType::INDEX) {
    buffer = Buffer::createIndexBuffer(logicalDevice, stagingBuffer.getSize());
  }

  buffer.copyBuffer(commandBuffer, stagingBuffer);
  return index;
}

GpuBufferManager::GpuBufferMapIndex GpuBufferManager::transferBuffer(Buffer&& stagingBuffer) {
  GpuBufferMapIndex index = getNextHandle(_bufferMap.size(), _freeBufferIndices);
  _bufferMap.insertUnsafe(*index, std::move(stagingBuffer));
  return index;
}

bool GpuBufferManager::removeBuffer(GpuBufferManager::GpuBufferMapIndex index) {
  if (!_bufferMap.exists(*index)) [[unlikely]] {
    return false;
  }

  _bufferMap.eraseUnsafe(*index);
  if (*index != _bufferMap.size()) [[unlikely]] {
    _freeBufferIndices.push_back(index);
  }

  return true;
}
