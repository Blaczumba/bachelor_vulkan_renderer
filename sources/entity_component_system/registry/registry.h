#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component_storage.h"
#include "entity_component_system/component/component.h"

#include <array>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <tuple>

using ComponentMask = std::bitset<MAX_COMPONENTS>;

class Registry {
    EntityManager entityManager;

    // std::unordered_map<ComponentType, std::unique_ptr<BaseStorage>> componentStorage;
    std::array<std::unique_ptr<BaseStorage>, MAX_COMPONENTS> componentStorage = {};
    std::unordered_map<Entity, ComponentMask> entityComponentMask;

public:
    Entity createEntity();

    template <typename Component>
    void addComponent(Entity entity, Component component) {
        constexpr ComponentType typeID = Component::getComponentID();

        if (!componentStorage[typeID]) {
            componentStorage[typeID] = std::make_unique<ComponentStorage<Component>>();
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
        storage->addComponent(entity, component);

        entityComponentMask[entity][typeID] = true;
    }

    template <typename... Components>
    void addComponents(Entity entity, Components... components) {
        auto& signature = entityComponentMask[entity];
        (addComponentHelper(entity, signature, components), ...);
    }

    template <typename Component>
    Component* getComponent(Entity entity) {
        constexpr ComponentType typeID = Component::getComponentID();
        if (entityComponentMask[entity][typeID]) {
            auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
            return storage->getComponent(entity);
        }
        return nullptr;
    }

    template<typename... Components>
    std::tuple<Components*...> getComponents(Entity entity) {
        const ComponentMask& componentMask = entityComponentMask[entity];
        std::tuple<Components*...> components = std::make_tuple(getComponentHelper<Components>(componentMask, entity)...);
        return components;
    }

    template <typename Component>
    void removeComponent(Entity entity) {
        constexpr ComponentType typeID = Component::getComponentID();
        auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
        storage->removeComponent(entity);
        entityComponentMask[entity][typeID] = false;
    }

    template <typename... Components>
    std::vector<Entity> getEntitiesWithComponents() {
        ComponentMask requiredMask;

        ((requiredMask.set(Components::getComponentID())), ...);

        std::vector<Entity> result;
        result.reserve(MAX_COMPONENTS);

        for (const auto& [entity, mask] : entityComponentMask) {
            if ((mask & requiredMask) == requiredMask) {
                result.emplace_back(entity);
            }
        }
        return result;
    }

private:
    template <typename Component>
    void addComponentHelper(Entity entity, ComponentMask& componentMask, Component component) {
        constexpr ComponentType typeID = Component::getComponentID();

        if (!componentStorage[typeID]) {
            componentStorage[typeID] = std::make_unique<ComponentStorage<Component>>();
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
        storage->addComponent(entity, component);

        componentMask[typeID] = true;
    }

    template<typename Component>
    Component* getComponentHelper(const ComponentMask& componentMask, Entity entity) {
        constexpr ComponentType typeID = Component::getComponentID();
        if (componentMask[typeID]) {
            auto* storage = static_cast<ComponentStorage<Component>*>(componentStorage[typeID].get());
            return storage->getComponent(entity);
        }
        return nullptr;
    }

};