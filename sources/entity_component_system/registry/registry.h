#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component.h"

#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <tuple>

template<typename... Components>
class Registry {
	EntityManager entityManager;
	std::tuple<std::vector<std::unique_ptr<Components>>...> componentsData;

public:
	Registry(size_t maxEntities) : entityManager(maxEntities) {
		(std::get<std::vector<std::unique_ptr<Components>>>(componentsData).resize(maxEntities), ...);
	}

	Entity createEntity() {
		return entityManager.createEntity();
	}

	void destroyEntity(Entity entity) {
		entityManager.destroyEntity(entity);
	}

	template<typename Component>
	void addComponent(Entity entity, std::unique_ptr<Component> component) {
		std::get<std::vector<std::unique_ptr<Component>>>(componentsData)[entity] = std::move(component);
	}

	template<typename Component>
	Component* getComponent(Entity entity) {
		return std::get<std::vector<std::unique_ptr<Component>>>(componentsData)[entity].get();
	}

	//template<typename... Comps>
	//std::tuple<std::vector<std::unique_ptr<Components>>*...> getComponentsData() {
	//	std::tuple<std::vector<std::unique_ptr<Components>>*...> outputTuple = std::make_tuple(&std::get<std::vector<std::unique_ptr<Components>>>(componentsData)...);
	//	return outputTuple;
	//}
};
