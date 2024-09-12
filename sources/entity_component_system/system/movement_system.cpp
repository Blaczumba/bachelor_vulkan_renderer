#include "movement_system.h"

#include <iostream>
#include <tuple>
#include <chrono>

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
    const auto& entities = registry.getEntities();

    Position* pos;
    Velocity* vel;

    for (Entity entity : entities) {
        pos = registry.getComponent<Position>(entity);
        vel = registry.getComponent<Velocity>(entity);
        // auto [pos, vel] = registry.getComponents<Position, Velocity>(entity);
        pos->x += vel->dx * deltaTime;
        pos->y += vel->dy * deltaTime;

        //std::cout << "Entity " << entity << " moved to ("
        //    << pos.x << ", " << pos.y << ")\n";
    }
}