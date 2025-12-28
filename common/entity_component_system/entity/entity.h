#pragma once

#include <bitset>
#include <cstdint>
#include <limits>

#include "lib/types/util.h"

constexpr size_t MAX_ENTITIES = 255;
using Entity = typename lib::SmallestIndex<MAX_ENTITIES>::type;

constexpr size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = uint8_t;

template <typename... Components>
Signature getSignature() {  // TODO: Change to constexpr in later c++.
  Signature signature;
  (signature.set(Components::getComponentID()), ...);
  return signature;
}
