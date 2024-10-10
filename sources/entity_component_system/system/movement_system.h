#pragma once

#include "system.h"

#include "entity_component_system/component/component.h"
#include "entity_component_system/registry/registry.h"

class MovementSystem : public System {
    Registry* registry;

public:
    MovementSystem(Registry* reg);

    void update(float deltaTime) override;
};