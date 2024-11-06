#pragma once

#include "entity.h"

#include <vector>

class EntityManager {
private:
    std::vector<Entity> availableEntities;

public:
    EntityManager(size_t maxEntities);

    Entity createEntity();
    void destroyEntity(Entity entity);
};