#pragma once

#include "component_storage.h"
#include "component.h"

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>

class Archetype {
	std::bitset<MAX_COMPONENTS> _signature;
	std::array<MAX_COMPONENTS, std::unique_ptr<BaseStorage>> _componentStorages;

public:
	Archetype(const std::bitset<MAX_COMPONENTS> signature);

	template<typename Component>
	ComponentStorage<Component>* getComponentStorage() const {
		constexpr ComponentType componentID = Component::getComponentID();
		return _componentStorages[componentID] ? _componentStorages[componentID].get() : nullptr;
	}
};
