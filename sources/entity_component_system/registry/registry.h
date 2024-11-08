#pragma once

#include "entity_component_system/component/component_pool.h"
#include "entity_component_system/entity/entity_manager.h"

#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>

class Registry {
	EntityManager entityManager;
	std::array<std::unique_ptr<ComponentPool>, MAX_COMPONENTS> componentsData;

public:
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
	void updateComponents(std::function<void(Components&...)> callback) {
		auto components = std::tuple<std::array<std::optional<Components>, MAX_ENTITIES>&...>(
			static_cast<ComponentPoolImpl<Components>*>(componentsData[Components::getComponentID()].get())->getComponents()...
		);

		Entity min = 0;
		Entity max = MAX_ENTITIES - 1;

		auto updateMinMax = [&min, &max](auto* componentPool) {
			auto [minEntity, maxEntity] = componentPool->getMinMax();
			min = std::max(min, minEntity);
			max = std::min(max, maxEntity);
		};

		(updateMinMax(static_cast<ComponentPoolImpl<Components>*>(componentsData[Components::getComponentID()].get())), ...);

		for (Entity e = min; e <= max; ++e) {
			if ((std::get<std::array<std::optional<Components>, MAX_ENTITIES>&>(components)[e].has_value() && ...)) {
				callback(*std::get<std::array<std::optional<Components>, MAX_ENTITIES>&>(components)[e]...);
			}
		}
	}
};
