#include "texture.h"

class Texture2DColor : public Texture {
public:
    Texture2DColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DColor();

};