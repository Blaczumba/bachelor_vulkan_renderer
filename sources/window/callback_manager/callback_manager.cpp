#include "window/window/window_glfw.h"
#include "window/callback_observer/callback_observer.h"

#include "callback_manager.h"


void CallbackManager::attach(CallbackObserver* observer) {
	_observers.push_back(observer);
}
