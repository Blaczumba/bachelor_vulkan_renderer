#pragma once

#include "physical_device/device/physical_device.h"
#include "swapchain/swapchain.h"
#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>

class Screenshot {
	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	Screenshot(std::shared_ptr<LogicalDevice> logicalDevice);
	~Screenshot();

	void saveImage(const std::string& filePath, const Image& image);

private:
	struct CommandData {
		Image srcImage;
		Image dstImage;
		std::string filePath;
	};

	uint32_t _counter;

	std::thread _savingThread;
	std::queue<CommandData> _commandData;
	std::mutex _commandDataMutex;
	std::condition_variable _commandDataCV;
	bool _shouldStop;

	void savingThread();
	void savePixels(const CommandData& imagesData);
};
