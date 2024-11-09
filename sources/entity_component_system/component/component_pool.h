#pragma once

#include "entity_component_system/entity/entity.h"

#include <optional>
#include <unordered_map>
#include <set>
#include <vector>

#include <boost/container/flat_set.hpp>

class ComponentPool {
public:
	virtual void destroyEntity(Entity entity) = 0;
	virtual ~ComponentPool() = default;
};

template<typename Component>
class ComponentPoolImpl : public ComponentPool {
	std::vector<Component> _components;
	boost::container::flat_set<Entity> _entities;		// TODO: Change to flat map.

public:
	~ComponentPoolImpl() override = default;

	void addComponent(Entity entity, Component&& component) {
		_entities.emplace(entity);
		_components.resize(*_entities.crbegin() + 1);
		_components[entity] = std::move(component);
	}

	void destroyEntity(Entity entity) override {
		_entities.erase(entity);
		_components.resize(std::max(*_entities.crbegin() - 1, 0));
		_components[entity] = Component{};
	}

	Component& getComponent(Entity entity) {
		return _components[entity];
	}

	std::vector<Component>& getComponents() {
		return _components;
	}

	std::pair<Entity, Entity> getMinMax() const {
		return std::make_pair(*_entities.cbegin(), *_entities.crbegin());
	}
};
