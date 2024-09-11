#include "movement_system.h"

#include <iostream>
#include <tuple>

MovementSystem::MovementSystem(Registry& reg) : registry(reg) {}

/*
    Registry registry;

    // Create entities
    Entity e1 = registry.createEntity();
    Entity e2 = registry.createEntity();

    // Add components
    registry.addComponent(e1, Position{ 0.0f, 0.0f });
    registry.addComponent(e1, Velocity{ 1.0f, 1.0f });

    registry.addComponent(e2, Position{ 10.0f, 10.0f });
    registry.addComponent(e2, Velocity{ 0.5f, -0.5f });

    // Create systems
    MovementSystem movementSystem(registry);

    // Run system logic
    for (int i = 0; i < 10; ++i) {
        movementSystem.update(1);
    }
*/
void MovementSystem::update(float deltaTime) {
    auto& entityComponentsData = registry.getEntityComponentsData<Position, Velocity>();

    for (auto& [entity, components] : entityComponentsData) {
        auto& [pos, vel] = components;

        pos.x += vel.dx * deltaTime;
        pos.y += vel.dy * deltaTime;

        //std::cout << "Entity " << entity << " moved to ("
        //    << pos->x << ", " << pos->y << ")\n";
    }
}