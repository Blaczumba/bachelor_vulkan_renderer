#pragma once

#include "vulkan/resource_manager/handle.h"
#include "vulkan/resource_manager/reference_counter.h"

template <typename Resource>
class Ref {
  Ref(ReferenceCounter<Resource>& counter, HandleFor<Resource> handle);

public:
  Ref(const Ref& other);

  Ref(Ref&& other) noexcept;

  Ref& operator=(const Ref& other);

  Ref& operator=(Ref&& other) noexcept;

  ~Ref();

private:
  ReferenceCounter<Resource>* _counter = nullptr;
  HandleFor<Resource> _handle;
};
