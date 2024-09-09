#pragma once

#include "entity_component_system/entity/entity.h"

#include <unordered_map>

class BaseStorage {
public:
    virtual ~BaseStorage() = default;
};

template <typename ComponentType>
class ComponentStorage : public BaseStorage {
    std::unordered_map<Entity, ComponentType> components;

public:
    void addComponent(Entity entity, ComponentType component) {
        components[entity] = component;
    }

    void removeComponent(Entity entity) {
        components.erase(entity);
    }

    ComponentType* getComponent(Entity entity) {
        if (components.find(entity) != components.end())
            return &components[entity];
        return nullptr;
    }
};

