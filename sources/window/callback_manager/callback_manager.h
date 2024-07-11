#pragma once

#include "window/window.h"

#include "window/callback_observer/callback_observer.h"

#include <vector>
#include <memory>

class CallbackManager {
protected:
	std::shared_ptr<Window> _window;

	std::vector<CallbackObserver*> _observers;
public:
	CallbackManager(std::shared_ptr<Window> window) : _window(window) {};
	void attach(CallbackObserver* observer);

	virtual void pollEvents() = 0;
};
