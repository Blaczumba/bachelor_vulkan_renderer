#include "texture.h"

class Texture2DDepth : public Texture {
public:
	Texture2DDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DDepth();
};