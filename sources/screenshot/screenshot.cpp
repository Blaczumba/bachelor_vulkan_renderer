#include "screenshot.h"

#include "command_buffer/command_buffer.h"
#include "memory_objects/image.h"

#include <fstream>

Screenshot::Screenshot(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Swapchain> swapchain)
	: _logicalDevice(logicalDevice), _swapchain(swapchain) {
	_blittingEnabled = physicalDevice->checkBlittingSupport(_swapchain->getSwapchainImageFormat()) &&
		physicalDevice->checkBlittingSupport(VK_FORMAT_R8G8B8A8_SRGB);
}

void Screenshot::saveImage(const std::string& filepath, VkImage srcImage, VkExtent2D extent) {
	// Source for the copy is the last rendered swapchain image
	VkDevice device = _logicalDevice->getVkDevice();
	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
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

		// Transition destination image to transfer destination layout
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition swapchain image from present to transfer source layout
		transitionImageLayout(copyCmd, srcImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
		if (_blittingEnabled)
		{
			// Define the region to blit (we will blit the whole swapchain image)
			VkOffset3D blitSize;
			blitSize.x = extent.width;
			blitSize.y = extent.height;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion{};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;

			// Issue the blit command
			vkCmdBlitImage(
				copyCmd,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
		}
		else
		{
			// Otherwise use image copy (requires us to manually flip components)
			VkImageCopy imageCopyRegion{};
			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.srcSubresource.layerCount = 1;
			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.dstSubresource.layerCount = 1;
			imageCopyRegion.extent.width = extent.width;
			imageCopyRegion.extent.height = extent.height;
			imageCopyRegion.extent.depth = 1;

			// Issue the copy command
			vkCmdCopyImage(
				copyCmd,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageCopyRegion);
		}

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition back the swap chain image after the blit is done
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
	// Check if source is BGR
	// Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
	if (!_blittingEnabled) {
		std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
		// colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), format) != formatsBGR.end());
	}

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


void Screenshot::saveScreenshot(const std::string& filepath, uint32_t imageIndex) {
	// Source for the copy is the last rendered swapchain image
	VkDevice device = _logicalDevice->getVkDevice();
	const VkFormat format = _swapchain->getSwapchainImageFormat();
	const VkExtent2D extent = _swapchain->getExtent();
	VkImage srcImage = _swapchain->getVkImage(imageIndex);
	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
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

		// Transition destination image to transfer destination layout
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition swapchain image from present to transfer source layout
		transitionImageLayout(copyCmd, srcImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
		if (_blittingEnabled)
		{
			// Define the region to blit (we will blit the whole swapchain image)
			VkOffset3D blitSize;
			blitSize.x = extent.width;
			blitSize.y = extent.height;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion{};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;

			// Issue the blit command
			vkCmdBlitImage(
				copyCmd,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
		}
		else
		{
			// Otherwise use image copy (requires us to manually flip components)
			VkImageCopy imageCopyRegion{};
			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.srcSubresource.layerCount = 1;
			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.dstSubresource.layerCount = 1;
			imageCopyRegion.extent.width = extent.width;
			imageCopyRegion.extent.height = extent.height;
			imageCopyRegion.extent.depth = 1;

			// Issue the copy command
			vkCmdCopyImage(
				copyCmd,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageCopyRegion);
		}

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		transitionImageLayout(copyCmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		// Transition back the swap chain image after the blit is done
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
	// Check if source is BGR
	// Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
	if (!_blittingEnabled) {
		std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
		colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), format) != formatsBGR.end());
	}

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
