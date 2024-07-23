#include "screenshot.h"

#include "command_buffer/command_buffer.h"
#include "memory_objects/image.h"

#include <algorithm>
#include <fstream>
#include <iostream>

Screenshot::Screenshot(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice), _counter(0), _shouldStop(false) {
	_savingThread = std::thread(&Screenshot::savingThread, this);
}

void Screenshot::addImageToObserved(const Image& image, const std::string& name) {
	_images.push_back(image);
	_imageNames.push_back(name);
}

void Screenshot::updateInput(const CallbackData& cbData) {
	if (std::find(cbData.keys.cbegin(), cbData.keys.cend(), Keyboard::Key::P) != cbData.keys.cend()) {
		for (size_t i = 0; i < _images.size(); i++) {
			saveImage(std::to_string(_counter++) + '_' + _imageNames[i], _images[i]);
		}
	}
}

Screenshot::~Screenshot() {
	{
		std::lock_guard<std::mutex> lck(_commandDataMutex);
		_shouldStop = true;
		_commandDataCV.notify_one();
	}

	if (_savingThread.joinable()) {
		_savingThread.join();
	}

	{
		CommandData commandData;
		std::lock_guard<std::mutex> lck(_commandDataMutex);
		while (!_commandData.empty()) {
			commandData = std::move(_commandData.front());
			_commandData.pop();
			freeImageResources(commandData.dstImage);
		}
	}
}

void Screenshot::savingThread() {
	CommandData commandData;
	while (!_shouldStop) {
		{
			std::unique_lock<std::mutex> lck(_commandDataMutex);
			_commandDataCV.wait(lck, [this]() { return !_commandData.empty() || _shouldStop; });
			if (_shouldStop) {
				continue;
			}
			commandData = std::move(_commandData.front());
			_commandData.pop();
		}
		savePixels(commandData);
	}
}

void Screenshot::saveImage(const std::string& filePath, const Image& srcImage) {
	VkDevice device = _logicalDevice->getVkDevice();
	VkExtent2D extent = { srcImage.extent.width, srcImage.extent.height };
	Image dstImage;
	dstImage.extent = { extent.width, extent.height, 1 };

	_logicalDevice->createImage(
		dstImage.extent.width,
		dstImage.extent.height,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		dstImage.image,
		dstImage.memory
	);

	{
		SingleTimeCommandBuffer commandBufferGuard(*_logicalDevice);
		VkCommandBuffer copyCmd = commandBufferGuard.getCommandBuffer();

		transitionImageLayout(copyCmd, dstImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(copyCmd, srcImage.image, srcImage.layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		copyImageToImage(copyCmd, srcImage.image, dstImage.image, extent);

		transitionImageLayout(copyCmd, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(copyCmd, srcImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImage.layout, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	{
		std::lock_guard<std::mutex> lck(_commandDataMutex);
		_commandData.push({ srcImage, dstImage, filePath });
		_commandDataCV.notify_one();
	}
}

void Screenshot::savePixels(const CommandData& imagesData) {
	const auto& dstImage = imagesData.dstImage;
	const auto& srcImage = imagesData.srcImage;
	const VkExtent2D extent = { dstImage.extent.width, dstImage.extent.height };

	VkDevice device = _logicalDevice->getVkDevice();

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout(device, dstImage.image, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(device, dstImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(imagesData.filePath, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << extent.width << "\n" << extent.height << "\n" << 255 << "\n";

	bool colorSwizzle = false;
	std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
	colorSwizzle = (std::find_if(formatsBGR.begin(), formatsBGR.end(), [&srcImage](VkFormat givenFormat) { return srcImage.format == givenFormat; }) != formatsBGR.end());

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

	vkUnmapMemory(device, dstImage.memory);
	freeImageResources(dstImage);
}

void Screenshot::freeImageResources(const Image& image) const {
	VkDevice device = _logicalDevice->getVkDevice();

	vkFreeMemory(device, image.memory, nullptr);
	vkDestroyImage(device, image.image, nullptr);
}
