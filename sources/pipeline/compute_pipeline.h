#include "pipeline.h"
#include <string>

class ComputePipeline : public Pipeline {
public:
	ComputePipeline(std::shared_ptr<LogicalDevice> logicalDevice, VkDescriptorSetLayout descriptorSetLayout, const std::string& computeShader);
	~ComputePipeline();

};