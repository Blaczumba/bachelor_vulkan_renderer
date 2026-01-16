#pragma once

#include <vector>

#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"

class BindlessDescriptorSetWriter {
  static constexpr size_t RESOURCE_COUNT = 256;

  using TextureHandleMap = lib::SparseMap<const Texture*, RESOURCE_COUNT>;
  using BufferHandleMap = lib::SparseMap<const Buffer*, RESOURCE_COUNT>;

  BindlessDescriptorSetWriter(const DescriptorSet& descriptorSet) noexcept;

public:
  DEFINE_STRONG_INT(TextureHandle, TextureHandleMap::IndexType);
  DEFINE_STRONG_INT(BufferHandle, BufferHandleMap::IndexType);

  static std::unique_ptr<BindlessDescriptorSetWriter> create(
      const DescriptorSet& descriptorSet) noexcept;

  TextureHandle storeTexture(const Texture& texture);
  std::vector<TextureHandle> storeTextures(std::span<const Texture> textures);
  void removeTexture(TextureHandle handle);

  BufferHandle storeBuffer(const Buffer& buffer);
  std::vector<BufferHandle> storeBuffers(std::span<const Buffer> buffers);
  void removeBuffer(BufferHandle handle);

private:
  const DescriptorSet& _descriptorSet;
  TextureHandleMap _texturesMap;
  std::vector<TextureHandle> _missingTextures;
  BufferHandleMap _buffersMap;
  std::vector<BufferHandle> _missingBuffers;
};

using BindlessTextureHandle = BindlessDescriptorSetWriter::TextureHandle;
using BindlessBufferHandle = BindlessDescriptorSetWriter::BufferHandle;
