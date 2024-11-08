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
	std::array<std::unique_ptr<ComponentPool>, MAX_COMPONENTS> _componentsData;
	std::array<Signature, MAX_ENTITIES> _signatures;

public:
	Entity createEntity() {
		return entityManager.createEntity();
	}

	void destroyEntity(Entity entity) {
		Signature i = 1;
		for (ComponentType j = 0; j < MAX_COMPONENTS; i <<= 2, ++j) {
			if ((_signatures[entity] & i) != 0) {
				_componentsData[j]->destroyEntity(entity);
			}
		}
		entityManager.destroyEntity(entity);
	}

	template<typename Component>
	void addComponent(Entity entity, Component&& component) {
		if (!_componentsData[Component::getComponentID()]) {
			_componentsData[Component::getComponentID()] = std::make_unique<ComponentPoolImpl<Component>>();
		}
		static_cast<ComponentPoolImpl<Component>*>(_componentsData[Component::getComponentID()].get())->addComponent(entity, std::move(component));
		_signatures[entity].set(Component::getComponentID());
	}

	template<typename Component>
	Component& getComponent(Entity entity) {
		if (!_componentsData[Component::getComponentID()]) {
			_componentsData[Component::getComponentID()] = std::make_unique<ComponentPoolImpl<Component>>();
		}
		_signatures[entity].set(Component::getComponentID());
		return static_cast<ComponentPoolImpl<Component>*>(_componentsData[Component::getComponentID()].get())->getComponent(entity);
	}

	template<typename... Components>
	std::tuple<Components&...> getComponents(Entity entity) {
		return std::tie(static_cast<ComponentPoolImpl<Components>*>(_componentsData[Components::getComponentID()].get())->getComponent(entity)...);
	}

	template<typename... Components>
	void updateComponents(std::function<void(Components&...)> callback) {
		Signature signature;
		(signature.set(Components::getComponentID()), ...);

		auto components = std::tuple<std::array<Components, MAX_ENTITIES>&...>(
			static_cast<ComponentPoolImpl<Components>*>(_componentsData[Components::getComponentID()].get())->getComponents()...
		);

		Entity min = 0;
		Entity max = MAX_ENTITIES - 1;

		auto updateMinMax = [&min, &max](auto* componentPool) {
			auto [minEntity, maxEntity] = componentPool->getMinMax();
			min = std::max(min, minEntity);
			max = std::min(max, maxEntity);
		};

		(updateMinMax(static_cast<ComponentPoolImpl<Components>*>(_componentsData[Components::getComponentID()].get())), ...);

		for (Entity e = min; e <= max; ++e) {
			if ((_signatures[e] & signature) == signature) {
				callback(std::get<std::array<Components, MAX_ENTITIES>&>(components)[e]...);
			}
		}
	}
};
