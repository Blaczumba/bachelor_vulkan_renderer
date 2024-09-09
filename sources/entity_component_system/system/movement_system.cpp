#include "movement_system.h"

#include <iostream>

MovementSystem::MovementSystem(Registry& reg) : registry(reg) {}

void MovementSystem::update(float deltaTime) {
    auto entities = registry.getEntitiesWithComponents<Position, Velocity>();

    for (auto entity : entities) {
        auto* pos = registry.getComponent<Position>(entity);
        auto* vel = registry.getComponent<Velocity>(entity);

        if (pos && vel) {
            pos->x += vel->dx * deltaTime;
            pos->y += vel->dy * deltaTime;

            std::cout << "Entity " << entity << " moved to ("
                << pos->x << ", " << pos->y << ")\n";
        }
    }
}