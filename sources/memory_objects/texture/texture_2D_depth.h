#include "texture.h"

class Texture2DDepth : public Texture {
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
public:
	Texture2DDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DDepth();

    VkImageView getVkImageView() const;
    VkSampler getVkSampler() const;
};