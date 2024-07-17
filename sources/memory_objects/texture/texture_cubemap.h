#include "texture_2D_sampler.h"
#include "logical_device/logical_device.h"

#include <string>

class TextureCubemap : public Texture2D {
	VkSampler _sampler;

	std::string _filePath;
	uint32_t _mipLevels = 1;
	float _samplerAnisotropy = 1.0f;
public:
	TextureCubemap(std::shared_ptr<LogicalDevice> logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy);
	~TextureCubemap();

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) override;
};
