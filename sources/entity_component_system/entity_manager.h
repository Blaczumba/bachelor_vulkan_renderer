#pragma once

#include "entity.h"

class EntityManager {
public:
	Entity createEntity();
	void destroyEntity(Entity entity);

private:
	Entity _nextEntity;
};
