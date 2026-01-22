#pragma once

#include "common/util/resource_handles.h"

#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <string>

namespace common {

template <typename AssetManagerImpl>
class AssetManager {
public:
  StagingImageDataResourceHandle loadImageAsync(const std::string& filePath) {
    return static_cast<AssetManagerImpl*>(this)->loadImageAsync(filePath);
  }

  template <typename Model, typename... Type>
  StagingVertexDataResourceHandle loadVertexDataInterleavingAsync(
      std::shared_ptr<Model>& modelPtr, std::span<const std::byte> indices, uint8_t indexSize,
      std::span<const std::pair<std::string, std::string>> orders,
      std::span<const Type>... attributes) {
    return static_cast<AssetManagerImpl*>(this)->loadVertexDataInterleavingAsync(
        modelPtr, indices, indexSize, orders, attributes...);
  }

  template <typename VertexType, typename Model>
  void loadVertexDataAsync(
      std::shared_ptr<Model>& modelPtr, const std::string& filePath,
      std::span<const std::byte> indices, uint8_t indexSize, std::span<const VertexType> vertices) {
    static_cast<AssetManagerImpl*>(this)->template loadVertexDataAsync<VertexType>(
        modelPtr, filePath, indices, indexSize, vertices);
  }
};

}  // namespace common
