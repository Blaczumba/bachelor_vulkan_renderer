#pragma once

#include <vector>
#include <memory>

class WindowGLFW;
class CallbackObserver;

class CallbackManager {
protected:
	std::shared_ptr<WindowGLFW> _window;

	std::vector<CallbackObserver*> _observers;
public:
	CallbackManager(std::shared_ptr<WindowGLFW> window) : _window(window) {};
	void attach(CallbackObserver* observer);

	virtual void pollEvents() = 0;
};
