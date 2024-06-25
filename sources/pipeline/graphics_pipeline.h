#pragma once

#include "pipeline.h"
#include "render_pass/render_pass.h"

#include <vulkan/vulkan.h>

#include <string>

class GraphicsPipeline : public Pipeline {
public:
	GraphicsPipeline(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Renderpass> renderpass, VkDescriptorSetLayout descriptorSetLayout, VkSampleCountFlagBits msaaSamples);
	~GraphicsPipeline();

};

