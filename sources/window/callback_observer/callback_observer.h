#pragma once

#include <vector>
#include <cstdint>

namespace Keyboard {
	enum class Key : uint32_t {
		NONE = 0,
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L, Z,
		X, C, V, B, N, M
	};
}

struct CallbackData {
	float deltaTime						= 0.0f;

	bool keyboardAction					= false;
	std::vector<Keyboard::Key> keys		= {};

	bool mouseAction					= false;
	float xoffset						= 0.0f;
	float yoffset						= 0.0f;
};

class CallbackObserver {
public:
	virtual void updateInput(const CallbackData& cbData) = 0;
};