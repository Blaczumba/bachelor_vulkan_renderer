#include "entity_manager.h"

Entity EntityManager::createEntity() {
    Entity entity = nextEntity++;
    entities.emplace_back(entity);
    return entity;
}

void EntityManager::destroyEntity(Entity entity) {
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
}

bool EntityManager::isAlive(Entity entity) {
    return std::find(entities.begin(), entities.end(), entity) != entities.end();
}