#pragma once

#include <memory>

#include "common/entity_component_system/entity/entity.h"
#include "common/util/geometry.h"
#include "common/util/resource_handles.h"

class MeshComponent {
  static constexpr ComponentType componentID = 2;

public:
  GpuBufferHandle vertexBufferHandle;
  GpuBufferHandle indexBufferHandle;
  GpuBufferHandle vertexBufferPrimitiveHandle;
  AABB aabb;
  VkIndexType indexType;

  static constexpr std::enable_if_t < componentID<MAX_COMPONENTS, ComponentType> getComponentID() {
    return componentID;
  }
};
