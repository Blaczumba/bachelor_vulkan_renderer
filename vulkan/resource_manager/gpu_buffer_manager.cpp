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

template <typename MapType, typename IndexType>
inline void increaseRefCountInternal(MapType& map, IndexType index) {
  if (auto* resource = map.tryGetValue(*index); resource != nullptr) {
    ++resource->refCount;
  }
}

template <typename MapType, typename IndexType>
inline void decreaseRefCountInternal(MapType& map, IndexType index) {
  auto* resource = map.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    return;
  }

  --resource->refCount;

  if (resource->refCount == 0) {
    map.eraseUnsafe(*index);
  }
}

}  // namespace

void GpuBufferManager::increaseRefCount(GpuBufferManager::GpuBufferMapIndex index) {
  increaseRefCountInternal(_bufferMap, index);
}

void GpuBufferManager::decreaseRefCount(GpuBufferManager::GpuBufferMapIndex index) {
  decreaseRefCountInternal(_bufferMap, index);
}

void GpuBufferManager::increaseRefCount(GpuBufferManager::GpuTextureMapIndex index) {
  increaseRefCountInternal(_textureMap, index);
}

void GpuBufferManager::decreaseRefCount(GpuBufferManager::GpuTextureMapIndex index) {
  decreaseRefCountInternal(_textureMap, index);
}

GpuBufferManager::GpuBufferMapIndex GpuBufferManager::uploadBuffer(
    VkCommandBuffer commandBuffer, const Buffer& stagingBuffer, BufferType bufferType) {
  const LogicalDevice& logicalDevice = stagingBuffer.getLogicalDevice();
  if (_bufferMap.size() == MAX_BUFFERS) [[unlikely]] {
    throw EngineException(
        "GpuBufferManager::uploadBuffer: Cannot upload more buffers, maximum limit reached.");
  }

  GpuBufferMapIndex index = getNextHandle(_bufferMap.size(), _freeBufferIndices);
  Buffer buffer = Buffer::createVertexInputBuffer(
        logicalDevice, stagingBuffer.getSize(), bufferType == BufferType::VERTEX
            ? VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT :
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  buffer.copyBuffer(commandBuffer, stagingBuffer);
  _bufferMap.insertUnsafe(*index, BufferResource(std::move(buffer), 1));
  return index;
}

GpuBufferManager::GpuBufferMapIndex GpuBufferManager::transferBuffer(Buffer&& stagingBuffer) {
  if (_bufferMap.size() == MAX_BUFFERS) [[unlikely]] {
    throw EngineException(
        "GpuBufferManager::uploadBuffer: Cannot upload more buffers, maximum limit reached.");
  }

  GpuBufferMapIndex index = getNextHandle(_bufferMap.size(), _freeBufferIndices);
  _bufferMap.insertUnsafe(*index, BufferResource(std::move(stagingBuffer), 1));
  return index;
}

const Buffer& GpuBufferManager::getBuffer(GpuBufferManager::GpuBufferMapIndex index) const {
  const BufferResource* resource = _bufferMap.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    throw EngineException(std::format(
        "GpuBufferManager::getBuffer: Buffer with index {} does not exist.", *index));
  }

  return resource->buffer;
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

GpuBufferManager::GpuTextureMapIndex GpuBufferManager::transferTexture(Texture&& texture) {
  GpuTextureMapIndex index = getNextHandle(_textureMap.size(), _freeTextureIndices);
  if (_textureMap.size() == MAX_TEXTURES) [[unlikely]] {
    throw EngineException(
        "GpuBufferManager::transferTexture: Cannot upload more textures, maximum limit reached.");
  }

  _textureMap.insertUnsafe(*index, TextureResource(std::move(texture), 1));
  return index;
}

const Texture& GpuBufferManager::getTexture(GpuBufferManager::GpuTextureMapIndex index) const {
  const TextureResource* resource = _textureMap.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    throw EngineException(std::format(
        "GpuBufferManager::getTexture: Texture with index {} does not exist.", *index));
  }

  return resource->texture;
}
