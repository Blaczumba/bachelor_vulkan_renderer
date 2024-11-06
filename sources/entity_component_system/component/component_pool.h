#pragma once

#include "entity_component_system/entity/entity.h"

#include <unordered_map>
#include <vector>

class ComponentPool {
public:
	virtual void destroyEntity(Entity entity) = 0;
	virtual ~ComponentPool() = default;
};

template<typename Component>
class ComponentPoolImpl : public ComponentPool {
	std::vector<Component> _components;
	std::unordered_map<Entity, size_t> _entityToIndex;	// TODO change to flat unordered map
	std::unordered_map<size_t, Entity> _indexToEntity;	// TODO change to flat unordered map

public:
	~ComponentPoolImpl() override = default;

	void addComponent(Entity entity, Component&& component) {
		_entityToIndex.emplace(entity, _components.size());
		_indexToEntity.emplace(_components.size(), entity);
		_components.emplace_back(std::move(component));
	}

	void destroyEntity(Entity entity) override {
		// TODO use fewer reads from map
		size_t lastIndex = _components.size() - 1;
		size_t indexToRemove = _entityToIndex[entity];
		_components[indexToRemove] = std::move(_components[lastIndex]);

		Entity lastEntity = _indexToEntity[lastIndex];
		_entityToIndex[lastEntity] = indexToRemove;
		_indexToEntity[indexToRemove] = lastEntity;

		_entityToIndex.erase(entity);
		_indexToEntity.erase(lastIndex);
	}

	Component& getComponent(Entity entity) {
		return _components[_entityToIndex[entity]];
	}

	std::unordered_map<Entity, Component>& getComponents() {
		return _components;
	}
};
