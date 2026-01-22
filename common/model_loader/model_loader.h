#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

#include "lib/buffer/buffer.h"
#include "lib/buffer/shared_buffer.h"
#include "common/util/resource_handles.h"

template <typename IndexT>
std::enable_if_t<std::is_unsigned<IndexT>::value, lib::Buffer<std::byte>> processIndices(
    std::span<const IndexT> srcIndices, uint8_t indexSize) {
  lib::Buffer<std::byte> indices(srcIndices.size() * indexSize);
  std::byte* data = indices.data();
  for (const IndexT& index : srcIndices) {
    std::memcpy(data, &index, indexSize);
    data += indexSize;
  }
  return indices;
}

struct ImageID {
  StagingImageDataResourceHandle ID;
  std::string path;
};

struct VertexData {
  lib::Buffer<glm::vec3> positions;
  uint8_t indexSize;

  glm::mat4 model;

  ImageID diffuseTexture;
  ImageID normalTexture;
  ImageID metallicRoughnessTexture;

  StagingVertexDataResourceHandle vertexResourceID;
};

class ModelLoader {
public:
  ~ModelLoader() = default;
};
