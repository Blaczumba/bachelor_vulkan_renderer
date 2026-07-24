#include "gpu_buffer_manager.h"

#include <format>
#include <memory>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "common/util/resource_handles.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/image.h"
#include "vulkan/wrapper/memory_objects/memory_objects_lib.h"

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
inline bool decreaseRefCountInternal(MapType& map, IndexType index) {
  auto* resource = map.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    return false;
  }

  --resource->refCount;

  if (resource->refCount == 0) {
    map.eraseUnsafe(*index);
    return true;
  }
  return false;
}

void copyBuffer(
    const VkCommandBuffer commandBuffer, VkBuffer dstBuffer, const BufferMetadata& dstMetadata,
    VkBuffer srcBuffer, const BufferMetadata& srcMetadata,
    std::optional<VkDeviceSize> srcSize = std::nullopt, VkDeviceSize srcOffset = 0,
    VkDeviceSize dstOffset = 0) {
  if ((dstMetadata.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) [[unlikely]] {
    throw EngineException(
        "When copying one buffer to other the destination one must have "
        "VK_BUFFER_USAGE_TRANSFER_DST_BIT specified.");
  }

  if ((srcMetadata.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) [[unlikely]] {
    throw EngineException(
        "When copying one buffer to other the source one must have "
        "VK_BUFFER_USAGE_TRANSFER_SRC_BIT specified.");
  }

  const VkDeviceSize size = srcSize.value_or(srcMetadata.size);
  if (srcOffset + size > srcMetadata.size) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        srcOffset, size, srcMetadata.size));
  }

  if (dstOffset + size > dstMetadata.size) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        dstOffset, size, dstMetadata.size));
  }

  copyBufferToBuffer(commandBuffer, srcBuffer, dstBuffer, srcOffset, dstOffset, size);
}

}  // namespace

std::unique_ptr<GpuBufferManager> GpuBufferManager::create() {
  return std::unique_ptr<GpuBufferManager>(new GpuBufferManager());
}

void GpuBufferManager::increaseRefCount(GpuBufferHandle index) {
  increaseRefCountInternal(_bufferMap, index);
}

void GpuBufferManager::decreaseRefCount(GpuBufferHandle index) {
  if (decreaseRefCountInternal(_bufferMap, index)) {
    _freeBufferIndices.push_back(index);
  }
}

void GpuBufferManager::increaseRefCount(GpuImageHandle index) {
  increaseRefCountInternal(_imageMap, index);
}

void GpuBufferManager::decreaseRefCount(GpuImageHandle index) {
  if (decreaseRefCountInternal(_imageMap, index)) {
    _freeImageIndices.push_back(index);
  }
}

GpuBufferHandle GpuBufferManager::storeBuffer(
    VkCommandBuffer commandBuffer, const BufferWithMetadata& stagingBuffer, BufferType bufferType) {
  const LogicalDevice& logicalDevice = stagingBuffer.buffer.getLogicalDevice();
  if (_bufferMap.size() == MAX_GPU_BUFFERS) [[unlikely]] {
    throw EngineException(std::format(
        "GpuBufferManager::uploadBuffer: Cannot upload more buffers, maximum limit of {} reached.",
        MAX_GPU_BUFFERS));
  }

  BufferBuilder bufferBuilder;
  Buffer buffer =
      bufferBuilder
          .withUsage(bufferType == BufferType::VERTEX ?
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT :
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
          .withSize(stagingBuffer.metadata.size)
          .createVertexInputBuffer(logicalDevice);
  copyBuffer(commandBuffer, buffer.getVkBuffer(), bufferBuilder.getMetadata(),
             stagingBuffer.buffer.getVkBuffer(), stagingBuffer.metadata);
  GpuBufferHandle index = getNextHandle(_bufferMap.size(), _freeBufferIndices);
  _bufferMap.insertUnsafe(
      *index,
      BufferResource(BufferWithMetadata{std::move(buffer), bufferBuilder.getMetadata()}, 1));
  return index;
}

GpuBufferHandle GpuBufferManager::transferBuffer(BufferWithMetadata&& stagingBuffer) {
  if (_bufferMap.size() == MAX_GPU_BUFFERS) [[unlikely]] {
    throw EngineException(
        std::format("GpuBufferManager::transferBuffer: Cannot upload more " "buffers, maximum "
                                                                            "limit " "of {} "
                                                                                     "reached.",
                    MAX_GPU_BUFFERS));
  }

  GpuBufferHandle index = getNextHandle(_bufferMap.size(), _freeBufferIndices);
  _bufferMap.insertUnsafe(*index, BufferResource(std::move(stagingBuffer), 1));
  return index;
}

const BufferWithMetadata& GpuBufferManager::getBuffer(GpuBufferHandle index) const {
  const BufferResource* resource = _bufferMap.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    throw EngineException(
        std::format("GpuBufferManager::getBuffer: Buffer with index {} does not exist.", *index));
  }

  return resource->buffer;
}

bool GpuBufferManager::removeBuffer(GpuBufferHandle index) {
  if (_bufferMap.erase(*index)) {
    _freeBufferIndices.push_back(index);
    return true;
  }
  return false;
}

GpuImageHandle GpuBufferManager::transferImage(Image&& image, const ImageMetadata& metadata) {
  GpuImageHandle index = getNextHandle(_imageMap.size(), _freeImageIndices);
  if (_imageMap.size() == MAX_GPU_IMAGES) [[unlikely]] {
    throw EngineException(std::format(
        "GpuBufferManager::transferImage: Cannot upload more images, maximum limit of {} reached.",
        MAX_GPU_IMAGES));
  }

  _imageMap.insertUnsafe(*index, ImageResource(std::make_tuple(std::move(image), metadata), 1));
  return index;
}

const std::tuple<Image, ImageMetadata>& GpuBufferManager::getImage(GpuImageHandle index) const {
  const ImageResource* resource = _imageMap.tryGetValue(*index);
  if (resource == nullptr) [[unlikely]] {
    throw EngineException(
        std::format("GpuBufferManager::getTexture: Texture with index {} does not exist.", *index));
  }

  return resource->image;
}

bool GpuBufferManager::removeImage(GpuImageHandle index) {
  if (!_imageMap.exists(*index)) [[unlikely]] {
    return false;
  }

  _imageMap.eraseUnsafe(*index);
  _freeImageIndices.push_back(index);

  return true;
}
