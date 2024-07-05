#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"
#include "render_pass/render_pass.h"
#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Framebuffer {
	std::vector<VkFramebuffer> _framebuffers;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<Swapchain> _swapchain;
	std::shared_ptr<Renderpass> _renderPass;

	std::vector<std::unique_ptr<Image>> _images;
public:
	Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::vector<std::vector<VkImageView>>&& images, std::shared_ptr<Renderpass> renderpass, VkExtent2D extent, uint32_t count);
	Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Renderpass> renderpass);
	~Framebuffer();

	std::vector<VkFramebuffer> getVkFramebuffers() const;
};
