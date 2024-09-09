#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component_storage.h"
#include "entity_component_system/component/component.h"

#include <unordered_map>
#include <memory>
#include <bitset>

constexpr size_t MAX_COMPONENTS = 64;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

class Registry {
    EntityManager entityManager;

    std::unordered_map<ComponentType, std::unique_ptr<BaseStorage>> componentStorage;
    std::unordered_map<Entity, ComponentMask> entityComponentMask;

public:
    Entity createEntity();

    template <typename Component>
    void addComponent(Entity entity, Component component) {
        constexpr auto typeID = Component::componentID;
        auto ptr = componentStorage.find(typeID);
        if (ptr == componentStorage.end()) {
            ptr = componentStorage.emplace(typeID, std::make_unique<ComponentStorage<Component>>()).first;
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(ptr->second.get());
        storage->addComponent(entity, component);

        entityComponentMask[entity][typeID] = true;
    }

    template <typename... Components>
    void addComponents(Entity entity, Components... components) {
        (addComponent(entity, components), ...);
    }

    template <typename Component>
    Component* getComponent(Entity entity) {
        constexpr auto typeID = Component::componentID;
        if (entityComponentMask[entity][typeID]) {
            auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
            return storage->getComponent(entity);
        }
        return nullptr;
    }

    template <typename Component>
    void removeComponent(Entity entity) {
        constexpr auto typeID = Component::componentID;
        auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
        storage->removeComponent(entity);
        entityComponentMask[entity][typeID] = false;
    }

    template <typename... Components>
    std::vector<Entity> getEntitiesWithComponents() {
        ComponentMask requiredMask;

        ((requiredMask[Components::componentID] = true), ...);

        std::vector<Entity> result;
        result.reserve(MAX_COMPONENTS);

        for (const auto& [entity, mask] : entityComponentMask) {
            if ((mask & requiredMask) == requiredMask) {
                result.emplace_back(entity);
            }
        }
        return result;
    }
};