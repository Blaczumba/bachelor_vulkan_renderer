#include "pipeline.h"

Pipeline::Pipeline(const LogicalDevice& logicalDevice, VkPipelineBindPoint pipelineBindPoint)
	: _logicalDevice(logicalDevice), _pipelineBindPoint(pipelineBindPoint) {

}

VkPipeline Pipeline::getVkPipeline() const {
	return _pipeline;
}

VkPipelineLayout Pipeline::getVkPipelineLayout() const {
	return _pipelineLayout;
}

VkPipelineBindPoint Pipeline::getVkPipelineBindPoint() const {
	return _pipelineBindPoint;
}