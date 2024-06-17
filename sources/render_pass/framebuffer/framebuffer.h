#include "logical_device/logical_device.h"
#include "swapchain/swapchain.h"
#include "render_pass/render_pass.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Framebuffer {
	std::vector<VkFramebuffer> _framebuffers;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<Swapchain> _swapchain;
	std::shared_ptr<Renderpass> _renderPass;
public:
	Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Renderpass> renderpass);
private:

	void createDepthResources(const VkAttachmentDescription& description, VkImage image, VkDeviceMemory imageMemory, VkImageView imageView);
};
