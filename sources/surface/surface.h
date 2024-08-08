#pragma once

#include <memory>

class Instance;
class Window;

class Surface {
	VkSurfaceKHR _surface;
	const std::shared_ptr<Window> _window;
	const std::shared_ptr<Instance> _instance;

public:
	Surface(const std::shared_ptr<Instance>& instance, const std::shared_ptr<Window>& window);
	~Surface();
	const VkSurfaceKHR getVkSurface() const;
};