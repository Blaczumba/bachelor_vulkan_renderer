#include "descriptor_set.h"

#include "logical_device/logical_device.h"
#include "pipeline/pipeline.h"
#include "memory_objects/uniform_buffer/uniform_buffer.h"
#include "descriptor_pool.h"
#include "descriptor_set_layout.h"

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stdexcept>

DescriptorSet::DescriptorSet(const std::shared_ptr<const DescriptorPool>& descriptorPool)
	: _descriptorPool(descriptorPool) {
    const VkDescriptorSetLayout layout = _descriptorPool->getDescriptorSetLayout().getVkDescriptorSetLayout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = _descriptorPool->getVkDescriptorPool();
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = &layout;

    if (vkAllocateDescriptorSets(_descriptorPool->getLogicalDevice().getVkDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

DescriptorSet::~DescriptorSet() {
    
}

void DescriptorSet::updateDescriptorSet(const std::vector<UniformBuffer*>& uniformBuffers) {

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    for (size_t j = 0; j < uniformBuffers.size(); j++) {
        const UniformBuffer* uniformBuffer = uniformBuffers[j];

        descriptorWrites.emplace_back(uniformBuffer->getVkWriteDescriptorSet(_descriptorSet, j));

        if (uniformBuffer->getVkDescriptorType() == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            _dynamicBuffersBaseSizes.emplace_back(uniformBuffer->getSize());
        }
    }

    vkUpdateDescriptorSets(_descriptorPool->getLogicalDevice().getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, const std::vector<uint32_t>& dynamicOffsetStrides) {
    std::vector<uint32_t> sizes;
    sizes.reserve(dynamicOffsetStrides.size());
    std::transform(dynamicOffsetStrides.cbegin(), dynamicOffsetStrides.cend(), _dynamicBuffersBaseSizes.cbegin(), std::back_inserter(sizes), std::multiplies<uint32_t>());

    vkCmdBindDescriptorSets(commandBuffer, pipeline.getVkPipelineBindPoint(), pipeline.getVkPipelineLayout(), 0, 1, &_descriptorSet, sizes.size(), sizes.data());
}

const VkDescriptorSet DescriptorSet::getVkDescriptorSet(size_t i) const {
    return _descriptorSet;
}
