#pragma once
#include "instance/instance.h"

#include "vulkan/vulkan.h"

#include <vector>

class Window {
protected:
	VkSurfaceKHR _surface;
	const Instance& _instance;

public:
	Window(const Instance& instance);
	virtual ~Window();

	const Instance& getInstance() const;

	virtual bool open() const =0;
	virtual void setWindowSize(int width, int height) =0;
	virtual VkExtent2D getFramebufferSize() const =0;
	const VkSurfaceKHR getVkSurfaceKHR() const;
};
