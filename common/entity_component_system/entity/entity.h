#pragma once

#include <bitset>
#include <cstdint>
#include <limits>

#include "lib/types/util.h"
#include "lib/types/strong_int.h"

constexpr size_t MAX_ENTITIES = 255;
DEFINE_STRONG_INT(Entity, typename lib::SmallestIndex<MAX_ENTITIES>::type);

constexpr size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = typename lib::SmallestIndex<MAX_COMPONENTS>::type;

template <typename... Components>
Signature getSignature() {  // TODO: Change to constexpr in later c++.
  Signature signature;
  (signature.set(Components::getComponentID()), ...);
  return signature;
}
