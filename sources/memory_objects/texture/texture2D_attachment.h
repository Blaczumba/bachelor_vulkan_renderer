#include "texture.h"

class Texture2DAttachment : public Texture {
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
public:
	Texture2DAttachment(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkExtent2D extent);
    ~Texture2DAttachment();

    VkImageView getVkImageView() const;
    VkSampler getVkSampler() const;
};