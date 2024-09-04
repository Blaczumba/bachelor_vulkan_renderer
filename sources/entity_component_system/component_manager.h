#pragma once

#include "entity.h"

#include <unordered_map>

class ComponentManager {
public:
    template<typename Component>
    void addComponent(Entity entity, Component component) {
        getComponentContainer<Component>()[entity] = component;
    }

    template<typename Component>
    Component& getComponent(Entity entity) {
        return getComponentContainer<Component>()[entity];
    }
    std::hash<int> a;
    template<typename Component>
    void removeComponent(Entity entity) {
        getComponentContainer<Component>().erase(entity);
    }

private:
    template<typename Component>
    std::unordered_map<Entity, Component>& getComponentContainer() {
        static std::unordered_map<Entity, Component> container;
        return container;
    }
};