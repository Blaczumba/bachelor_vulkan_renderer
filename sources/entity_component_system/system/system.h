#pragma once

// System base class
class System {
public:
    virtual void update(float deltaTime) = 0;
};