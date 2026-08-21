#include "vulkan/resource_manager/ref.h"

#include <utility>

#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
Ref<Resource>::Ref(ReferenceCounter<Resource>& counter, HandleFor<Resource> handle)
  : _counter(&counter), _handle(handle) {
  _counter->incrementRefCount(_handle);
}

template <typename Resource>
Ref<Resource>::Ref(const Ref& other) : _counter(other._counter), _handle(other._handle) {
  if (_counter) {
    _counter->incrementRefCount(_handle);
  }
}

template <typename Resource>
Ref<Resource>::Ref(Ref&& other) noexcept
  : _counter(std::exchange(other._counter, nullptr)), _handle(other._handle) {}

template <typename Resource>
Ref<Resource>& Ref<Resource>::operator=(const Ref& other) {
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

template <typename Resource>
Ref<Resource>& Ref<Resource>::operator=(Ref&& other) noexcept {
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

template <typename Resource>
Ref<Resource>::~Ref() {
  if (_counter) {
    _counter->decrementRefCount(_handle);
  }
}

template <typename Resource>
HandleFor<Resource> Ref<Resource>::getHandle() const noexcept {
  return _handle;
}

template class Ref<Sampler>;
