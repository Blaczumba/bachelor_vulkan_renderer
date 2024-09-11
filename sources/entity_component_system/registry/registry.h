#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component_storage.h"
#include "entity_component_system/component/component.h"

#include "entity_component_system/component/archetype.h"

#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <tuple>

class Registry {
    EntityManager entityManager;

    std::unordered_map<Signature, std::unique_ptr<BaseArchetype>> archetypes;

public:
    template<typename... Components>
    Entity createEntity(const Components&... components) {
        Entity entity = entityManager.createEntity();
        Signature signature = createSignature<Components...>();
        auto ptr = archetypes.find(signature);
        if (ptr != archetypes.cend()) {
            auto archetype = static_cast<Archetype<Components...>*>(ptr->second.get());
            archetype->addEntity(entity, components...);
        }
        else {
            auto archetype = std::make_unique<Archetype<Components...>>();
            archetype->addEntity(entity, components...);
            archetypes.emplace(signature, std::move(archetype));
        }
        return entity;
    }

    template<typename... Components>
    std::tuple<Components...>& getComponents(Entity entity) {
        return getEntityComponentsData<Components...>()[entity];
    }

#ifdef BOOST_ENABLED
    template<typename... Components>
    boost::unordered_flat_map<Entity, std::tuple<Components...>>& getEntityComponentsData() {
        return static_cast<Archetype<Components...>*>(archetypes[createSignature<Components...>()].get())->getData();
    }
#else
    std::unordered_map<Entity, std::tuple<Components...>>& getEntityComponentsData() {
        return archetypes[createSignature<Components...>()].getData<Components...>();
    }
#endif // BOOST_ENABLED

};