#pragma once

#include <vector>

class CallbackObserver;

class CallbackManager {
protected:
	std::vector<CallbackObserver*> _observers;

public:
	CallbackManager() = default;
	void attach(CallbackObserver* observer);

	virtual void pollEvents() = 0;
};
