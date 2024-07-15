#pragma once

#include "window/window/window_glfw.h"

#include "window/callback_observer/callback_observer.h"

#include <vector>
#include <memory>

class CallbackManager {
protected:
	std::shared_ptr<WindowGLFW> _window;

	std::vector<CallbackObserver*> _observers;
public:
	CallbackManager(std::shared_ptr<WindowGLFW> window) : _window(window) {};
	void attach(CallbackObserver* observer);

	virtual void pollEvents() = 0;
};
