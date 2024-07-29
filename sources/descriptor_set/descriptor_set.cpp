#include "descriptor_set.h"

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stdexcept>

DescriptorSet::DescriptorSet(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, VkDescriptorPool descriptorPool)
	: _logicalDevice(logicalDevice), _descriptorSetLayout(descriptorSetLayout) {

    VkDescriptorSetLayout layout = _descriptorSetLayout->getVkDescriptorSetLayout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = descriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = &layout;

    if (vkAllocateDescriptorSets(_logicalDevice->getVkDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

DescriptorSet::~DescriptorSet() {
    
}

void DescriptorSet::updateDescriptorSet(const std::vector<std::shared_ptr<UniformBuffer>>& uniformBuffers, uint32_t dynamicOffsetCount) {

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    _offsets = std::vector<std::vector<uint32_t>>(dynamicOffsetCount);

    for (size_t j = 0, dynUniformIndex = 0; j < uniformBuffers.size(); j++) {
        const auto& uniformBuffer = uniformBuffers[j];

        descriptorWrites.emplace_back(uniformBuffer->getVkWriteDescriptorSet(_descriptorSet, j));

        if (uniformBuffer->getVkDescriptorType() == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            _dynamicBuffersBaseSizes.emplace_back(uniformBuffer->getSize());
        }
    }

    vkUpdateDescriptorSets(_logicalDevice->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    for(uint32_t index = 0; index < dynamicOffsetCount; ++index)
        std::transform(_dynamicBuffersBaseSizes.cbegin(), _dynamicBuffersBaseSizes.cend(), std::back_inserter(_offsets[index]), [index](uint32_t baseSize) { return index * baseSize; });
}

void DescriptorSet::bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, uint32_t dynamicOffsetStride) {
    uint32_t size = static_cast<uint32_t>(_offsets[dynamicOffsetStride].size());
    const uint32_t* dynamicOffsetPointer = (size) ? _offsets[dynamicOffsetStride].data() : nullptr;
    vkCmdBindDescriptorSets(commandBuffer, pipeline.getVkPipelineBindPoint(), pipeline.getVkPipelineLayout(), 0, 1, &_descriptorSet, size, dynamicOffsetPointer);
}

VkDescriptorSetLayout DescriptorSet::getVkDescriptorSetLayout() const {
    return _descriptorSetLayout->getVkDescriptorSetLayout();
}

VkDescriptorSet& DescriptorSet::getVkDescriptorSet(size_t i) {
    return _descriptorSet;
}
