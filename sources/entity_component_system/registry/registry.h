#pragma once

#include "entity_component_system/entity/entity_manager.h"
#include "entity_component_system/component/component_storage.h"

#include <unordered_map>
#include <memory>
#include <bitset>

constexpr size_t MAX_COMPONENTS = 64;
using ComponentType = std::size_t;

class Registry {
    EntityManager entityManager;

    std::unordered_map<ComponentType, std::unique_ptr<BaseStorage>> componentStorage;
    std::unordered_map<Entity, std::bitset<MAX_COMPONENTS>> entityComponentMask;

public:
    Entity createEntity();

    // Add component to entity
    template <typename T>
    void addComponent(Entity entity, T component) {
        constexpr auto typeID = T::componentID;
        auto ptr = componentStorage.find(typeID);
        if (ptr == componentStorage.end()) {
            ptr = componentStorage.emplace(typeID, std::make_unique<ComponentStorage<T>>()).first;
        }

        auto* storage = static_cast<ComponentStorage<T>*>(ptr->second.get());
        storage->addComponent(entity, component);

        entityComponentMask[entity][typeID] = true;
    }

    // Get a component from entity
    template <typename T>
    T* getComponent(Entity entity) {
        constexpr auto typeID = T::componentID;
        if (entityComponentMask[entity][typeID]) {
            auto* storage = static_cast<ComponentStorage<T>*>(componentStorage[typeID].get());
            return storage->getComponent(entity);
        }
        return nullptr;
    }

    // Remove component from entity
    template <typename T>
    void removeComponent(Entity entity) {
        constexpr auto typeID = T::componentID;
        auto* storage = static_cast<ComponentStorage<T>*>(componentStorage[typeID].get());
        storage->removeComponent(entity);
        entityComponentMask[entity][typeID] = false;
    }

    // Query entities that have all specified components
    template <typename T, typename... Ts>
    std::vector<Entity> getEntitiesWithComponents() {
        std::bitset<MAX_COMPONENTS> requiredMask;

        // Ustawienie bitów dla komponentu T
        requiredMask[T::componentID] = true;

        // Ustawienie bitów dla pozosta³ych komponentów (Ts...)
        ((requiredMask[Ts::componentID] = true), ...);

        std::vector<Entity> result;

        for (const auto& [entity, mask] : entityComponentMask) {
            // SprawdŸ, czy maska encji zawiera wszystkie wymagane komponenty
            if ((mask & requiredMask) == requiredMask) {
                result.push_back(entity);
            }
        }
        return result;
    }
};