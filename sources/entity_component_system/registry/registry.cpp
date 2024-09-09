#include "registry.h"

Entity Registry::createEntity() {
    Entity entity = entityManager.createEntity();
    entityComponentMask.emplace(entity, ComponentMask());
    return entity;
}
