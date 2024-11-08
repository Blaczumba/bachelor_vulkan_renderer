#pragma once

#include "entity_component_system/entity/entity.h"

#include <optional>
#include <unordered_map>
#include <set>
#include <vector>

class ComponentPool {
public:
	virtual void destroyEntity(Entity entity) = 0;
	virtual ~ComponentPool() = default;
};

template<typename Component>
class ComponentPoolImpl : public ComponentPool {
  std::array<Component, MAX_ENTITIES> _components;
	std::set<Entity> _entities;		// TODO: Change to flat map.

public:
	~ComponentPoolImpl() override = default;

	void addComponent(Entity entity, Component&& component) {
		_components[entity] = std::move(component);
		_entities.emplace(entity);
	}

	void destroyEntity(Entity entity) override {
		_components[entity] = Component{};
		_entities.erase(entity);
	}

	Component& getComponent(Entity entity) {
		return _components[entity];
	}

	std::array<Component, MAX_ENTITIES>& getComponents() {
		return _components;
	}

	std::pair<Entity, Entity> getMinMax() const {
		return std::make_pair(*_entities.begin(), *_entities.rbegin());
	}
};
