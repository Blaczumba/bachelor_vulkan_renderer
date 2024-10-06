#include "entity_manager.h"

#include <algorithm>
#include <iterator>

EntityManager::EntityManager(size_t maxEntities) {
    availableEntities.reserve(maxEntities);
    std::generate_n(std::back_inserter(availableEntities), maxEntities, [val = 0]() mutable { return val++; });
}

Entity EntityManager::createEntity() {
    Entity entity = availableEntities.back();
    usedEntities.push_back(entity);
    availableEntities.pop_back();
    return entity;
}

void EntityManager::destroyEntity(Entity entity) {
    auto ptr = std::find(usedEntities.cbegin(), usedEntities.cend(), entity);
    if (ptr != usedEntities.cend()) {
        availableEntities.push_back(*ptr);
        usedEntities.erase(ptr);
    }
}

const std::vector<Entity>& EntityManager::getUsedEntities() const {
    return usedEntities;
}
