#pragma once

#include "memory_objects/texture/image.h"

#include <ktx.h>
#include <stb_image/stb_image.h>
#include <vulkan/vulkan.h>

#include <span>
#include <string_view>
#include <vector>
#include <variant>

struct ImageResource {
	std::variant<stbi_uc*, ktxTexture*> libraryResource;
	ImageDimensions dimensions;
	void* data;
	size_t size;
};

class ImageLoader {
public:
	static ImageResource loadCubemapImage(std::string_view imagePath);
	static ImageResource load2DImage(std::string_view imagePath);
	static void deallocateResources(ImageResource& resource);
};
