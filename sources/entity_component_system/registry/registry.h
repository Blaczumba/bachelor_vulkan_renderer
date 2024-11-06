#pragma once

#include "entity_component_system/component/component.h"
#include "entity_component_system/component/component_pool.h"
#include "entity_component_system/entity/entity_manager.h"

#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <tuple>
#include <unordered_map>

class Archetype {

};

template<typename... Components>
class ArchetypeImpl : public Archetype {
	std::tuple<std::vector<Components>...> _components;

public:
	// void addEntity
};

class Registry {
	EntityManager entityManager;
	std::array<std::unique_ptr<ComponentPool>, MAX_COMPONENTS> componentsData;
	// std::unordered_map<Signature, Archetype> _archetypes;

public:
	Registry(size_t maxEntities) : entityManager(maxEntities) {}

	Entity createEntity() {
		return entityManager.createEntity();
	}

	void destroyEntity(Entity entity) {
		for (size_t i = 0; i < MAX_COMPONENTS; i++)
			componentsData[i]->destroyEntity(entity);
		entityManager.destroyEntity(entity);
	}

	template<typename Component>
	void addComponent(Entity entity, Component&& component) {
		if (!componentsData[Component::getComponentID()]) {
			componentsData[Component::getComponentID()] = std::make_unique<ComponentPoolImpl<Component>>();
		}
		static_cast<ComponentPoolImpl<Component>*>(componentsData[Component::getComponentID()].get())->addComponent(entity, std::move(component));
	}

	template<typename Component>
	Component& getComponent(Entity entity) {
		if (!componentsData[Component::getComponentID()]) {
			componentsData[Component::getComponentID()] = std::make_unique<ComponentPoolImpl<Component>>();
		}
		return static_cast<ComponentPoolImpl<Component>*>(componentsData[Component::getComponentID()].get())->getComponent(entity);
	}

	template<typename... Components>
	std::tuple<Components&...> getComponents(Entity entity) {
		return std::tie(static_cast<ComponentPoolImpl<Components>*>(componentsData[Components::getComponentID()].get())->getComponent(entity)...);
	}

	template<typename... Components>
	void updateComponents(std::function<void(Components&...)>&& callback) {
		// TODO implement the logic and add archetypes
	}
};
