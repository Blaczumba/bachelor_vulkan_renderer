#pragma once

#include "entity_component_system/entity/entity.h"

#include <unordered_map>

class ComponentPool {
public:
};

template<typename Component>
class ComponentPoolImpl : public ComponentPool {
	std::unordered_map<Entity, Component> _components;	// TODO change to flat unordered map

public:
	void addComponent(Entity entity, const Component& component) {
		_components.emplace(entity, component);
	}

	void removeComponent(Entity entity) {
		_components.erase(entity);
	}

	Component& getComponent(Entity entity) {
		return _components[entity];
	}
};
