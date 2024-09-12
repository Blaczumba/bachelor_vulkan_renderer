#pragma once

#include "entity.h"

#include <vector>

class EntityManager {
private:
    std::vector<Entity> availableEntities;
    std::vector<Entity> usedEntities;

public:
    EntityManager(size_t maxEntities);

    Entity createEntity();
    void destroyEntity(Entity entity);
    const std::vector<Entity>& getUsedEntities() const;
};