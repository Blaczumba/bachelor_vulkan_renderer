#include "memory_objects/image.h"

class ImageColor : public Image {
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	ImageColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
	~ImageColor();
};