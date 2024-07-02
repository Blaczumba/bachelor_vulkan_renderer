#pragma once

#include "window/window.h"

#include <memory>

class CallbackManager {
protected:
	std::shared_ptr<Window> _window;
public:
	CallbackManager(std::shared_ptr<Window> window) : _window(window) {};
	virtual void pollEvents() =0;
};
