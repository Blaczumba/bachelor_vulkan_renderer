#include "entity_manager.h"

#include <iterator>
#include <numeric>

EntityManager::EntityManager() {
  _availableEntities.resize(MAX_ENTITIES);
  std::iota(_availableEntities.rbegin(), _availableEntities.rend(), Entity{0});
}

Entity EntityManager::createEntity() {
  const Entity entity = _availableEntities.back();
  _availableEntities.pop_back();
  return entity;
}

void EntityManager::destroyEntity(Entity entity) {
  _availableEntities.push_back(entity);
}
