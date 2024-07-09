#pragma once

#include <cstdint>

struct KeyboardData {
	float deltaTime;

	uint32_t key;
};

struct MouseData {
	float deltaTime;

	float xoffset;
	float yoffset;
};

class CallbackObserver {
public:
	virtual void updateKeyboard(const KeyboardData& cbData) = 0;
	virtual void updateMouse(const MouseData& cbData) = 0;
};