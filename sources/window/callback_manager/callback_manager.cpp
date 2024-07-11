#include "callback_manager.h"


void CallbackManager::attach(CallbackObserver* observer) {
	_observers.push_back(observer);
}
