#include "movement_system.h"

#include <chrono>
#include <iostream>
#include <tuple>

MovementSystem::MovementSystem(Registry* reg) : registry(reg) {}

void MovementSystem::update(float deltaTime) {
    registry->updateComponents<Position, Velocity>(
        [deltaTime](Position& pos, Velocity& vel) {
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;

            //std::cout << "Entity " << entity << " moved to ("
            //    << pos.x << ", " << pos.y << ")\n"; 
        }
    );
}