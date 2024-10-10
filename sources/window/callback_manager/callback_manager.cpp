#include "callback_manager.h"

#include "window/callback_observer/callback_observer.h"
#include "window/window/window_glfw.h"

void CallbackManager::attach(CallbackObserver* observer) {
	_observers.push_back(observer);
}
