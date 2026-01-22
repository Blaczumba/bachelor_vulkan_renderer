#pragma once

#include <vector>

#include "common/util/resource_handles.h"
#include "lib/sparse/sparse_map.h"
#include "lib/types/strong_int.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"

class BindlessDescriptorSetWriter {
  using TextureHandleMap = lib::SparseMap<const Texture*, MAX_UNIFORM_RESOURCES>;
  using BufferHandleMap = lib::SparseMap<const Buffer*, MAX_UNIFORM_RESOURCES>;

  BindlessDescriptorSetWriter(const DescriptorSet& descriptorSet) noexcept;

public:
  static std::unique_ptr<BindlessDescriptorSetWriter> create(
      const DescriptorSet& descriptorSet) noexcept;

  UniformTextureHandle storeTexture(const Texture& texture);
  std::vector<UniformTextureHandle> storeTextures(std::span<const Texture> textures);
  void removeTexture(UniformTextureHandle handle);

  UniformBufferHandle storeBuffer(const Buffer& buffer);
  std::vector<UniformBufferHandle> storeBuffers(std::span<const Buffer> buffers);
  void removeBuffer(UniformBufferHandle handle);

private:
  const DescriptorSet& _descriptorSet;
  TextureHandleMap _texturesMap;
  std::vector<UniformTextureHandle> _missingTextures;
  BufferHandleMap _buffersMap;
  std::vector<UniformBufferHandle> _missingBuffers;
};
