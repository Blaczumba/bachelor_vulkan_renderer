#pragma once

#include <cstdint>

namespace common {

class Ref {
  struct VTable {
    void (*incrementRefCount)(void* resourceManager, uint64_t handle);
    void (*decrementRefCount)(void* resourceManager, uint64_t handle);
  };

  // One VTable instance per (ResourceManager, Handle) pair, shared by all Refs
  // built from that pair. Returned by pointer so every Ref stores just 8 bytes.
  template <typename ResourceManager, typename Handle>
  static const VTable* vtableFor() {
    static constexpr VTable table{
      [](void* resourceManager, uint64_t handle) {
        static_cast<ResourceManager*>(resourceManager)
            ->incrementRefCount(static_cast<Handle>(handle));
      },
      [](void* resourceManager, uint64_t handle) {
        static_cast<ResourceManager*>(resourceManager)
            ->decrementRefCount(static_cast<Handle>(handle));
      },
    };
    return &table;
  }

public:
  Ref() noexcept = default;

  template <typename ResourceManager, typename Handle>
  Ref(ResourceManager* resourceManager, Handle handle)
    : _resourceManager(resourceManager), _vtable(vtableFor<ResourceManager, Handle>()),
      _handle(static_cast<uint64_t>(*handle)) {
    if (_resourceManager) {
      _vtable->incrementRefCount(_resourceManager, _handle);
    }
  }

  Ref(const Ref& other);

  Ref(Ref&& other) noexcept;

  Ref& operator=(const Ref& other);

  Ref& operator=(Ref&& other) noexcept;

  ~Ref();

  uint64_t getHandle() const noexcept {
    return _handle;
  }

private:
  void* _resourceManager = nullptr;
  const VTable* _vtable = nullptr;
  uint64_t _handle = 0;
};

}  // namespace common
