#pragma once

#include "entity_component_system/component/component.h"
#include "entity_component_system/entity/entity_manager.h"

#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <tuple>
#include <unordered_map>

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
		for (size_t i = 0; i < MAX_COMPONENTS; i++)
			if(componentsData[i][entity])
				componentsData[i][entity].reset();
	}

	const std::vector<Entity>& getEntities() const {
		return entityManager.getUsedEntities();
	}

	template<typename Component>
	void addComponent(Entity entity, std::unique_ptr<Component> component) {
		componentsData[Component::getComponentID()][entity] = std::move(component);
	}

	template<typename Component>
	Component& getComponent(Entity entity) {
		return static_cast<Component&>(*componentsData[Component::getComponentID()][entity]);
	}

	template<typename... Components>
	std::tuple<Components&...> getComponents(Entity entity) {
		return std::tie((*static_cast<Components*>(componentsData[Components::getComponentID()][entity].get()))...);
	}

	template<typename... Components>
	void updateComponents(std::function<void(Components&...)> callback) {
		const auto& entities = entityManager.getUsedEntities();
		for (const auto entity : entities)
			callback((static_cast<Components&>(*componentsData[Components::getComponentID()][entity]))...);
			// callback((*static_cast<Components*>(componentsData[Components::getComponentID()][entity].get()))...);
	}
};
