#include "entity_manager.h"

#include <algorithm>
#include <iterator>

EntityManager::EntityManager(size_t maxEntities) {
    availableEntities.reserve(maxEntities);
    std::generate_n(std::back_inserter(availableEntities), maxEntities, [val = 0]() mutable { return val++; });
}

Entity EntityManager::createEntity() {
    Entity entity = availableEntities.back();
    availableEntities.pop_back();
    return entity;
}

void EntityManager::destroyEntity(Entity entity) {
    availableEntities.push_back(entity);
}
