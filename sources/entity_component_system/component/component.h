#pragma once

#include <bitset>
#include <unordered_map>

constexpr size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = uint64_t;

class Position {
    static constexpr ComponentType componentID = 0;

public:
    float x, y;
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    static constexpr std::enable_if_t<componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};

class Velocity {
    static constexpr ComponentType componentID = 1;

public:
    float dx, dy;
    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}

    static constexpr std::enable_if_t<componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};
