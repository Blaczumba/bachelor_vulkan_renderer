#include "screenshot.h"

#include "command_buffer/command_buffer.h"
#include "memory_objects/image.h"

#include <fstream>

Screenshot::Screenshot(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice) {
}

void Screenshot::saveImage(const std::string& filepath, VkImage srcImage, VkExtent2D extent, VkFormat format) {
	// Source for the copy is the last rendered swapchain image
	VkDevice device = _logicalDevice->getVkDevice();
	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
	_logicalDevice->createImage(
		extent.width,
		extent.height,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		format,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		dstImage,
		dstImageMemory
	);

	{
		SingleTimeCommandBuffer commandBufferGuard(_logicalDevice.get());
		VkCommandBuffer copyCmd = commandBufferGuard.getCommandBuffer();

		// Transition destination image to transfer destination layout
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition swapchain image from present to transfer source layout
		transitionImageLayout(copyCmd, srcImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// copyImageToImage(copyCmd, srcImage, dstImage, extent);
		copyImageToImage(copyCmd, srcImage, dstImage, extent);

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition back the swap chain image after the copy is done
		transitionImageLayout(copyCmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	}

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(filepath, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << extent.width << "\n" << extent.height << "\n" << 255 << "\n";

	// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
	bool colorSwizzle = false;
	std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
	colorSwizzle = (std::find_if(formatsBGR.begin(), formatsBGR.end(), [format](VkFormat givenFormat) { return format == givenFormat; }) != formatsBGR.end());

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


void Screenshot::saveImage(const std::string& filePath, const Image& image, const VkExtent2D& extent) {
	// Source for the copy is the last rendered swapchain image
	VkDevice device = _logicalDevice->getVkDevice();
	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
	_logicalDevice->createImage(
		extent.width,
		extent.height,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		image.format,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		dstImage,
		dstImageMemory
	);

	{
		SingleTimeCommandBuffer commandBufferGuard(_logicalDevice.get());
		VkCommandBuffer copyCmd = commandBufferGuard.getCommandBuffer();

		// Transition destination image to transfer destination layout
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition swapchain image from present to transfer source layout
		transitionImageLayout(copyCmd, image.image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// copyImageToImage(copyCmd, srcImage, dstImage, extent);
		copyImageToImage(copyCmd, image.image, dstImage, extent);

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition back the swap chain image after the copy is done
		transitionImageLayout(copyCmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, 1);

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

	// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
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