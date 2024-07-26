#include "descriptor_pool.h"

DescriptorPool::DescriptorPool(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, uint32_t maxNumSets)
	: _logicalDevice(logicalDevice), _descriptorSetLayout(descriptorSetLayout), _maxNumSets(maxNumSets), _allocatedSets(0) {

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto [descriptorType, numOccurances] : _descriptorSetLayout->getDescriptorTypeCounter()) {
		poolSizes.emplace_back(VkDescriptorPoolSize{
				.type = descriptorType,
				.descriptorCount = _maxNumSets * numOccurances,
			}
		);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = _maxNumSets;

	if (vkCreateDescriptorPool(_logicalDevice->getVkDevice(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(_logicalDevice->getVkDevice(), _descriptorPool, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorPool::createDesriptorSet() {
	return std::make_unique<DescriptorSet>(_logicalDevice, _descriptorSetLayout, _descriptorPool);
}
