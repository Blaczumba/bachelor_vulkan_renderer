#pragma once

#include <bitset>
#include <cstdint>
#include <limits>

using Entity = uint16_t;
constexpr size_t MAX_ENTITIES = std::numeric_limits<Entity>::max();
constexpr size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = uint8_t;