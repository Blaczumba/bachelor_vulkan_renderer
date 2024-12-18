#pragma once

#include "memory_objects/texture/image.h"

#include <vulkan/vulkan.h>

#include <span>
#include <string_view>
#include <vector>

class ImageLoader {
public:
	static ImageResource loadCubemapImage(const std::string_view imagePath);
	static ImageResource load2DImage(const std::string_view imagePath);
};
