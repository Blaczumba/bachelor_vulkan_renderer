#pragma once

#include "component_storage.h"
#include "component.h"

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <tuple>

#ifdef BOOST_ENABLED
	#include <boost/unordered/unordered_flat_map.hpp>
#endif // BOOST_ENABLED

using Signature = std::bitset<MAX_COMPONENTS>;

template<typename... Components>
class Archetype {
	using ComponentTuple = std::tuple<std::vector<Components>...>;

	Signature _signature;
	ComponentTuple _componentData;
	std::vector<Entity> _entities;
	#ifdef BOOST_ENABLED
		boost::unordered_flat_map<Entity, std::tuple<Components...>> data;
	#else
		std::unordered_map<Entity, std::tuple<Components...>> data;
	#endif // BOOST_ENABLED

	static constexpr size_t ENTITY_CAPACITY = 100000;
public:

	Archetype() {
		(_signature.set(Components::getComponentID()), ...);
		/*_entities.reserve(ENTITY_CAPACITY);
		(std::get<std::vector<Components>>(_componentData).reserve(ENTITY_CAPACITY), ...);*/
	}

	void addEntity(Entity entity, const Components&... components) {
		/*_entities.push_back(entity);
		(std::get<std::vector<Components>>(_componentData).emplace_back(components), ...);*/
		data.emplace(entity, std::make_tuple(components...));
	}

	void addEntity(Entity entity, Components&&... components) {
		/*_entities.push_back(entity);
		(std::get<std::vector<Components>>(_componentData).emplace_back(components), ...);*/
		data.emplace(entity, std::make_tuple(components...));
	}

	void removeEntity(Entity entity) {
		data.erase(entity);
		/*auto index = std::distance(_entities.cbegin(), std::find(_entities.cbegin(), _entities.cend(), entity));
		if(index != _entities.size())
			(std::get<std::vector<Components>>(_componentData).erase(std::get<std::vector<Components>>(_componentData).begin() + index), ...);*/
	}

	std::tuple<Components...>* getComponents(Entity entity) {
		auto ptr = data.find(entity);
		if (ptr != data.cend()) {
			return &ptr->second;
		}
		return nullptr;
		/*auto index = std::distance(_entities.cbegin(), std::find(_entities.cbegin(), _entities.cend(), entity));
		return std::make_tuple(&std::get<std::vector<Components>>(_componentData)[index]...);*/
	}

	Signature getSignature() const {
		return _signature;
	}
};
