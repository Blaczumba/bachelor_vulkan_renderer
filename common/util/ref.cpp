#include "ref.h"

#include <utility>

namespace common {

Ref::Ref(const Ref& other)
  : _resourceManager(other._resourceManager), _vtable(other._vtable), _handle(other._handle) {
  if (_resourceManager) {
    _vtable->incrementRefCount(_resourceManager, _handle);
  }
}

Ref::Ref(Ref&& other) noexcept
  : _resourceManager(std::exchange(other._resourceManager, nullptr)), _vtable(other._vtable),
    _handle(other._handle) {}

Ref& Ref::operator=(const Ref& other) {
  if (this == &other) {
    return *this;
  }
  if (_resourceManager) {
    _vtable->decrementRefCount(_resourceManager, _handle);
  }
  _resourceManager = other._resourceManager;
  _vtable = other._vtable;
  _handle = other._handle;
  if (_resourceManager) {
    _vtable->incrementRefCount(_resourceManager, _handle);
  }
  return *this;
}

Ref& Ref::operator=(Ref&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (_resourceManager) {
    _vtable->decrementRefCount(_resourceManager, _handle);
  }
  _resourceManager = std::exchange(other._resourceManager, nullptr);
  _vtable = other._vtable;
  _handle = other._handle;
  return *this;
}

Ref::~Ref() {
  if (_resourceManager) {
    _vtable->decrementRefCount(_resourceManager, _handle);
  }
}

}  // namespace common
