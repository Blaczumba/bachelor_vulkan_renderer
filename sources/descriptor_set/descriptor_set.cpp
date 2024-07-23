#include "descriptor_set.h"

#include <unordered_map>
#include <algorithm>
#include <stdexcept>

DescriptorSets::DescriptorSets(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<std::vector<std::shared_ptr<UniformBuffer>>>& uniformBuffers)
	: _logicalDevice(logicalDevice) {

    checkInputDataCoherence(uniformBuffers);

    std::unordered_map<VkDescriptorType, uint8_t> descriptorTypeOccurances;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(uniformBuffers[0].size());

    for (uint32_t i = 0; i < layoutBindings.size(); ++i) {
        VkDescriptorType descriptorType = uniformBuffers[0][i]->getVkDescriptorType();
        VkShaderStageFlags stageFlags   = uniformBuffers[0][i]->getVkShaderStageFlags();

        layoutBindings[i].binding               = i;
        layoutBindings[i].descriptorCount       = 1;
        layoutBindings[i].descriptorType        = descriptorType;
        layoutBindings[i].pImmutableSamplers    = nullptr;
        layoutBindings[i].stageFlags            = stageFlags;

        descriptorTypeOccurances[descriptorType]++;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(_logicalDevice->getVkDevice(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    const uint32_t numSets = uniformBuffers.size();

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto [descriptorType, numOccurances] : descriptorTypeOccurances) {
        poolSizes.emplace_back(VkDescriptorPoolSize{
                .type               = descriptorType,
                .descriptorCount    = numSets * numOccurances,
            }
        );
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount  = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes     = poolSizes.data();
    poolInfo.maxSets        = numSets;

    if (vkCreateDescriptorPool(_logicalDevice->getVkDevice(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    std::vector<VkDescriptorSetLayout> layouts(numSets, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = _descriptorPool;
    allocInfo.descriptorSetCount    = numSets;
    allocInfo.pSetLayouts           = layouts.data();

    _descriptorSets.resize(numSets);
    if (vkAllocateDescriptorSets(_logicalDevice->getVkDevice(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < numSets; i++) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        for (size_t j = 0; j < uniformBuffers[i].size(); j++) {
            descriptorWrites.emplace_back(uniformBuffers[i][j]->getVkWriteDescriptorSet(_descriptorSets[i], j));
        }

        vkUpdateDescriptorSets(_logicalDevice->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

DescriptorSets::~DescriptorSets() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout DescriptorSets::getVkDescriptorSetLayout() const {
    return _descriptorSetLayout;
}

VkDescriptorSet& DescriptorSets::getVkDescriptorSet(size_t i) {
    return _descriptorSets[i];
}

bool DescriptorSets::checkInputDataCoherence(const std::vector<std::vector<std::shared_ptr<UniformBuffer>>>& uniformBuffers) const {
    bool coherent = true;

    const auto& firstRow = uniformBuffers.at(0);
    size_t size = firstRow.size();
    bool allRowsEqualSize = std::all_of(uniformBuffers.cbegin(), uniformBuffers.cend(), [size](const std::vector<std::shared_ptr<UniformBuffer>>& row) { return row.size() == size; });
    
    if (!allRowsEqualSize) {
        throw std::runtime_error("All rows of uniform buffer matrix must be equal in size!");
    }

    for (size_t i = 0; i < size; ++i) {
        VkDescriptorType descriptorType = firstRow[i]->getVkDescriptorType();
        VkShaderStageFlags stageFlags   = firstRow[i]->getVkShaderStageFlags();
        
        bool equalColumsTraits = std::all_of(uniformBuffers.cbegin(), uniformBuffers.cend(), [=](const std::vector<std::shared_ptr<UniformBuffer>>& row) { return row[i]->getVkDescriptorType() == descriptorType && row[i]->getVkShaderStageFlags() == stageFlags; });
        if (!equalColumsTraits) {
            throw std::runtime_error("All rows of uniform buffer matrix must have the same layout (descriptor types and stage flags - comparing by columns)!");
        }
    }

    return true;
}