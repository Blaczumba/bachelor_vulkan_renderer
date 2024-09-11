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

class BaseArchetype {

};

template<typename... Components>
class Archetype : public BaseArchetype {
	#ifdef BOOST_ENABLED
		boost::unordered_flat_map<Entity, std::tuple<Components...>> data;
	#else
		std::unordered_map<Entity, std::tuple<Components...>> data;
	#endif // BOOST_ENABLED

public:

	void addEntity(Entity entity, const Components&... components) {
		data.emplace(entity, std::make_tuple(components...));
	}

	void addEntity(Entity entity, Components&&... components) {
		data.emplace(entity, std::make_tuple(components...));
	}

	void removeEntity(Entity entity) {
		data.erase(entity);
	}

	std::tuple<Components...>* getComponents(Entity entity) {
		auto ptr = data.find(entity);
		if (ptr != data.cend()) {
			return &ptr->second;
		}
		return nullptr;
	}

#ifdef BOOST_ENABLED
	boost::unordered_flat_map<Entity, std::tuple<Components...>>& getData() {
		return data;
	}
#else
	std::unordered_map<Entity, std::tuple<Components...>>& getData() {
		return data;
	}
#endif // BOOST_ENABLED

};
