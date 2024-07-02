#include "texture.h"

class Texture2DColor : public Texture {
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
public:
    Texture2DColor(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DColor();

    VkImageView getVkImageView() const;
    VkSampler getVkSampler() const;
};