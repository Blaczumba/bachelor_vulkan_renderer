#pragma once

#include "entity.h"

#include <vector>

class EntityManager {
private:
    std::vector<Entity> entities;
    Entity nextEntity = 0;

public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isAlive(Entity entity);
};