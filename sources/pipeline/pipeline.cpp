#include "pipeline.h"

Pipeline::Pipeline(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice) {

}

VkPipeline Pipeline::getVkPipeline() const {
	return _graphicsPipeline;
}

VkPipelineLayout Pipeline::getVkPipelineLayout() const {
	return _pipelineLayout;
}