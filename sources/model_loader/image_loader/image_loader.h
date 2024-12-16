#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <string_view>
#include <vector>

class ImageLoader {
public:
	struct Image {
		uint32_t width;
		uint32_t height;
		void* data;
		size_t size;
		uint32_t mipLevels;
		uint32_t layerCount;
		std::vector<VkBufferImageCopy> copyRegions;
	};

	static Image loadCubemapImage(const std::string_view imagePath);
	static Image load2DImage(const std::string_view imagePath);
};
