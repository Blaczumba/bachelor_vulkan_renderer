#pragma once

#include "memory_objects/texture/image.h"

#include <vulkan/vulkan.h>

#include <span>
#include <string_view>
#include <vector>

class ImageLoader {
public:
	static ImageResource loadCubemapImage(std::string_view imagePath);
	static ImageResource load2DImage(std::string_view imagePath);
};
