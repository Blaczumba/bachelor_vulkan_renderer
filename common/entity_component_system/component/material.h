#pragma once

#include "common/entity_component_system/entity/entity.h"
#include "common/util/bindless_descriptor_handles.h"

class MaterialComponent {
  static constexpr ComponentType componentID = 3;

public:
  uint32_t diffuse;
  uint32_t normal;
  uint32_t metallicRoughness;

  static constexpr std::enable_if_t < componentID<MAX_COMPONENTS, ComponentType> getComponentID() {
    return componentID;
  }
};
