#include "entity_manager.h"

Entity EntityManager::createEntity() {
	return _nextEntity++;
}


void EntityManager::destroyEntity(Entity entity) {

}