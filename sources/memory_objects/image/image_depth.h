#include "memory_objects/image.h"

#include <memory>

class ImageDepth : protected Image {
	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	ImageDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
	~ImageDepth();
};