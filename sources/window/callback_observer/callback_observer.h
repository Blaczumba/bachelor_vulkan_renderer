#pragma once

#include <vector>
#include <cstdint>

enum class Direction : uint32_t {
	NONE = 0,
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT
};

struct CallbackData {
	float deltaTime						= 0.0f;

	bool keyboardAction					= false;
	std::vector<uint32_t> keys			= {};
	std::vector<Direction> directions	= {};

	bool mouseAction					= false;
	float xoffset						= 0.0f;
	float yoffset						= 0.0f;
};

class CallbackObserver {
public:
	virtual void updateInput(const CallbackData& cbData) = 0;
};