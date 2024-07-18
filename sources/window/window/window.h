#pragma once

#include "vulkan/vulkan.h"

#include <vector>

class Window {
	virtual VkSurfaceKHR createSurface(VkInstance instance) const =0;

protected:
	static std::vector<const char*> _windowExtensions;

public:
	virtual ~Window() = default;
	static const std::vector<const char*>& getWindowExtensions();

	virtual bool open() const =0;
	virtual void setWindowSize(int width, int height) =0;
	virtual VkExtent2D getFramebufferSize() const =0;

	friend class Surface;
};
