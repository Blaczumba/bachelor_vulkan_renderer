#pragma once

#include <memory>

#include "common/entity_component_system/entity/entity.h"
#include "common/util/geometry.h"

class MeshComponent {
  static constexpr ComponentType componentID = 2;

public:
  size_t vertexBufferHandle;
  size_t indexBufferHandle;
  size_t vertexBufferPrimitiveHandle;
  AABB aabb;
  VkIndexType indexType;

  static constexpr std::enable_if_t < componentID<MAX_COMPONENTS, ComponentType> getComponentID() {
    return componentID;
  }
};
