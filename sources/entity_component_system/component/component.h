#pragma once

#include <bitset>
#pragma once

#include <unordered_map>

class Component {
public:
    virtual ~Component() = default;
};

struct Position : Component {
    float x, y;
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    static constexpr uint64_t componentID = 0;
};

struct Velocity : Component {
    float dx, dy;
    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}

    static constexpr uint64_t componentID = 4;
};
