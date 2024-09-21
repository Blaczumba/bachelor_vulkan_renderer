#include "pipeline.h"

#include "logical_device/logical_device.h"

#include <string>

class ComputePipeline : public Pipeline {
	const LogicalDevice& _logicalDevice;

public:
	ComputePipeline(const LogicalDevice& logicalDevice, VkDescriptorSetLayout descriptorSetLayout, const std::string& computeShader);
	~ComputePipeline();

};