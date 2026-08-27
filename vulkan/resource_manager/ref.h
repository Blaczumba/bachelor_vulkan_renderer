#pragma once

#include <utility>

#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
class Ref {
public:
  Ref() noexcept = default;

  Ref(ReferenceCounter<Resource>& counter, HandleFor<Resource> handle)
    : _counter(&counter), _handle(handle) {
    _counter->incrementRefCount(_handle);
  }

  Ref(const Ref& other) : _counter(other._counter), _handle(other._handle) {
    if (_counter) {
      _counter->incrementRefCount(_handle);
    }
  }

  Ref(Ref&& other) noexcept
    : _counter(std::exchange(other._counter, nullptr)), _handle(other._handle) {}

  Ref& operator=(const Ref& other) {
    if (this == &other) {
      return *this;
    }

    if (_counter) {
      _counter->decrementRefCount(_handle);
    }

    _counter = other._counter;
    _handle = other._handle;

    if (_counter) {
      _counter->incrementRefCount(_handle);
    }
    return *this;
  }

  Ref& operator=(Ref&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    if (_counter) {
      _counter->decrementRefCount(_handle);
    }

    _counter = std::exchange(other._counter, nullptr);
    _handle = other._handle;

    return *this;
  }

  ~Ref() {
    if (_counter) {
      _counter->decrementRefCount(_handle);
    }
  }

  HandleFor<Resource> getHandle() const noexcept {
    return _handle;
  }

  ReferenceCounter<Resource>* getCounter() const noexcept {
    return _counter;
  }

private:
  ReferenceCounter<Resource>* _counter = nullptr;
  HandleFor<Resource> _handle;
};
