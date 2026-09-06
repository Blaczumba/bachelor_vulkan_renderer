#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <string>
#include <tuple>

#include "common/buffer/buffer.h"
#include "common/model_loader/image_loader/image_loader.h"
#include "common/model_loader/image_loader/types.h"
#include "common/util/resource_handles.h"

namespace common {

class AssetManager {
public:
  virtual StagingImageDataResourceHandle loadImageAsync(
      std::function<std::tuple<ImageResource, OwnedImageData>(void)>&& imageFunction) = 0;

  virtual StagingImageDataResourceHandle loadImageAsync(
      std::shared_ptr<void> modelPtr, ImageResource&& imageResource) = 0;

  virtual StagingVertexDataResourceHandle loadVertexDataInterleavingAsync(
      std::shared_ptr<void> modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::vector<BufferDescription>&& bufferDescriptions) = 0;
};

}  // namespace common
