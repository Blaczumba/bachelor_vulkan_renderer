#pragma once

#include "window/callback_observer/callback_observer.h"

#include <vulkan/vulkan.h>

#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>

class CallbackObserver;
class LogicalDevice;
struct Image;
struct CallbackData;

class Screenshot : public CallbackObserver {
	const LogicalDevice& _logicalDevice;

	std::vector<Image> _images;
	std::vector<std::string> _imageNames;

public:
	Screenshot(const LogicalDevice& logicalDevice);
	~Screenshot();

	void updateInput(const CallbackData& cbData) override;
	void addImageToObserved(const Image& image, const std::string& name);
	void saveImage(const std::string& filePath, Image* image);

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
	void freeImageResources(const Image& image) const;
};
