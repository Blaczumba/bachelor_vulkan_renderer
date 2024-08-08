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

DescriptorSet::DescriptorSet(const LogicalDevice& logicalDevice, const DescriptorPool& descriptorPool)
	: _logicalDevice(logicalDevice) {

    const VkDescriptorSetLayout layout = descriptorPool.getDescriptorSetLayout().getVkDescriptorSetLayout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = descriptorPool.getVkDescriptorPool();
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = &layout;

    if (vkAllocateDescriptorSets(_logicalDevice.getVkDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

DescriptorSet::~DescriptorSet() {
    
}

void DescriptorSet::updateDescriptorSet(const std::vector<UniformBuffer*>& uniformBuffers, uint32_t dynamicOffsetCount) {

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    _offsets = std::vector<std::vector<uint32_t>>(dynamicOffsetCount, std::vector<uint32_t>());

    for (size_t j = 0; j < uniformBuffers.size(); j++) {
        const UniformBuffer* uniformBuffer = uniformBuffers[j];

        descriptorWrites.emplace_back(uniformBuffer->getVkWriteDescriptorSet(_descriptorSet, j));

        if (uniformBuffer->getVkDescriptorType() == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            _dynamicBuffersBaseSizes.emplace_back(uniformBuffer->getSize());
        }
    }

    vkUpdateDescriptorSets(_logicalDevice.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    for(uint32_t index = 0; index < dynamicOffsetCount; ++index)
        std::transform(_dynamicBuffersBaseSizes.cbegin(), _dynamicBuffersBaseSizes.cend(), std::back_inserter(_offsets[index]), [index](uint32_t baseSize) { return index * baseSize; });
}

void DescriptorSet::bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, uint32_t dynamicOffsetStride) {
    const uint32_t size = static_cast<uint32_t>(_offsets[dynamicOffsetStride].size());
    const uint32_t* dynamicOffsetPointer = (size) ? _offsets[dynamicOffsetStride].data() : nullptr;
    vkCmdBindDescriptorSets(commandBuffer, pipeline.getVkPipelineBindPoint(), pipeline.getVkPipelineLayout(), 0, 1, &_descriptorSet, size, dynamicOffsetPointer);
}

const VkDescriptorSet DescriptorSet::getVkDescriptorSet(size_t i) const {
    return _descriptorSet;
}
