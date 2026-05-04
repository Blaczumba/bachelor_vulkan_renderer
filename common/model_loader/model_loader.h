#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "common/util/resource_handles.h"
#include "lib/buffer/buffer.h"

namespace common {

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

}  // namespace common
