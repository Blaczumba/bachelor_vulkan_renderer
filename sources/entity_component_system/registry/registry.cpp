#include "registry.h"

Entity Registry::createEntity() {
    Entity entity = entityManager.createEntity();
    entityComponentMask.emplace(entity, std::bitset<MAX_COMPONENTS>());
    return entity;
}
