#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <string>

#include "common/buffer/buffer.h"
#include "common/util/resource_handles.h"

namespace common {

class AssetManager {
public:
  virtual StagingImageDataResourceHandle loadImageAsync(const std::string& filePath) = 0;

  virtual StagingVertexDataResourceHandle loadVertexDataInterleavingAsync(
      std::shared_ptr<void> modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::vector<BufferDescription>&& bufferDescriptions) = 0;
};

}  // namespace common
