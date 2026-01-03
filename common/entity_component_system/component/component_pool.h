#pragma once

#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#include "common/entity_component_system/entity/entity.h"
#include "lib/sparse/sparse_map.h"

class ComponentPool {
public:
  virtual void destroyEntity(Entity entity) = 0;
  virtual ~ComponentPool() = default;
};

template <typename Component>
class ComponentPoolImpl : public ComponentPool {
  lib::SparseMap<Component, MAX_ENTITIES> _sparseMap;

public:
  ~ComponentPoolImpl() override = default;

  void addComponent(Entity entity, Component&& component) {
    _sparseMap.insertUnsafe(entity, std::move(component));
  }

  void destroyEntity(Entity entity) override {
    _sparseMap.eraseUnsafe(entity);
  }

  Component& getComponent(Entity entity) {
    return _sparseMap.getValue(entity);
  }

  std::span<Component> getComponents() {
    return _sparseMap.getValues();
  }
};
