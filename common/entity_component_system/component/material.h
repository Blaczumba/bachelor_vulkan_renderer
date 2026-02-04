#pragma once

#include "common/entity_component_system/entity/entity.h"
#include "common/util/resource_handles.h"

class MaterialComponent {
  static constexpr ComponentType componentID = 3;

public:
  UniformTextureHandle diffuse;
  UniformTextureHandle normal;
  UniformTextureHandle metallicRoughness;

  static constexpr std::enable_if_t < componentID<MAX_COMPONENTS, ComponentType> getComponentID() {
    return componentID;
  }
};
