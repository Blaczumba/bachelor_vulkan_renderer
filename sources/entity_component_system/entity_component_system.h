#pragma once

#include "entity_manager.h"
#include "component_manager.h"

class EntityComponentSystem {
    Entity createEntity();
    void destroyEntity(Entity entity);

    template<typename Component>
    void addComponent(Entity entity, Component component) {
        componentManager.addComponent(entity, component);
    }

    template<typename Component>
    Component& getComponent(Entity entity) {
        return componentManager.getComponent<Component>(entity);
    }

    template<typename Component>
    void removeComponent(Entity entity) {
        componentManager.removeComponent<Component>(entity);
    }

    void update(float deltaTime);

private:
    EntityManager entityManager;
    ComponentManager componentManager;
    std::vector<Entity> entities;
};
