#include "entity_component_system.h"

Entity EntityComponentSystem::createEntity() {
    return entityManager.createEntity();
}

void EntityComponentSystem::destroyEntity(Entity entity) {
    entityManager.destroyEntity(entity);
}

void EntityComponentSystem::update(float deltaTime) {

}
