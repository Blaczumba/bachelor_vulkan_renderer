#include "pipeline.h"

class ComputePipeline : public Pipeline {
public:
	ComputePipeline(std::shared_ptr<LogicalDevice> logicalDevice, VkDescriptorSetLayout descriptorSetLayout, const std::string& computeShader);
	~ComputePipeline();

};