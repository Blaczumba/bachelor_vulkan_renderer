#include "screenshot.h"

#include "command_buffer/command_buffer.h"
#include "memory_objects/image.h"

#include <fstream>
#include <iostream>

Screenshot::Screenshot(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice) {
}

void Screenshot::saveImage(const std::string& filePath, const Image& image) {
	// Source for the copy is the last rendered swapchain image
	VkDevice device = _logicalDevice->getVkDevice();
	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
	VkExtent2D extent = { image.extent.width, image.extent.height };

	_logicalDevice->createImage(
		extent.width,
		extent.height,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		dstImage,
		dstImageMemory
	);

	{
		SingleTimeCommandBuffer commandBufferGuard(_logicalDevice.get());
		VkCommandBuffer copyCmd = commandBufferGuard.getCommandBuffer();

		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(copyCmd, image.image, image.layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		copyImageToImage(copyCmd, image.image, dstImage, extent);

		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(copyCmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.layout, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(filePath, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << extent.width << "\n" << extent.height << "\n" << 255 << "\n";

	bool colorSwizzle = false;
	std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
	colorSwizzle = (std::find_if(formatsBGR.begin(), formatsBGR.end(), [&image](VkFormat givenFormat) { return image.format == givenFormat; }) != formatsBGR.end());

	// ppm binary pixel data
	for (uint32_t y = 0; y < extent.height; y++) {
		unsigned int* row = (unsigned int*)data;
		for (uint32_t x = 0; x < extent.width; x++) {
			if (colorSwizzle) {
				file.write((char*)row + 2, 1);
				file.write((char*)row + 1, 1);
				file.write((char*)row, 1);
			}
			else {
				file.write((char*)row, 3);
			}
			row++;
		}
		data += subResourceLayout.rowPitch;
	}

	vkUnmapMemory(device, dstImageMemory);
	vkFreeMemory(device, dstImageMemory, nullptr);
	vkDestroyImage(device, dstImage, nullptr);
}
