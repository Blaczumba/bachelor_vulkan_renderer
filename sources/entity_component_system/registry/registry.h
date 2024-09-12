#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component.h"

#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <tuple>

class Registry {
	EntityManager entityManager;
	std::array<std::vector<std::unique_ptr<Component>>, MAX_COMPONENTS> componentsData;

public:
	Registry(size_t maxEntities) : entityManager(maxEntities) {
		for (auto& el : componentsData)
			el.resize(maxEntities);
	}

	Entity createEntity() {
		return entityManager.createEntity();
	}

	void destroyEntity(Entity entity) {
		entityManager.destroyEntity(entity);
	}

	const std::vector<Entity>& getEntities() const {
		return entityManager.getUsedEntities();
	}

	template<typename Component>
	void addComponent(Entity entity, std::unique_ptr<Component> component) {
		componentsData[Component::getComponentID()][entity] = std::move(component);
	}

	template<typename Component>
	Component* getComponent(Entity entity) {
		return static_cast<Component*>(componentsData[Component::getComponentID()][entity].get());
	}

	template<typename... Components>
	std::tuple<std::vector<std::unique_ptr<Components>>*...> getComponentsDataPointers() {
		return std::make_tuple(reinterpret_cast<std::vector<std::unique_ptr<Components>>*>(&componentsData[Components::getComponentID()])...);
	}
};
