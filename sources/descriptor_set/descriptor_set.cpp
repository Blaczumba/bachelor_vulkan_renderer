#include "descriptor_set.h"

#include <unordered_map>
#include <stdexcept>

DescriptorSets::DescriptorSets(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>>& uniformBuffers)
	: _logicalDevice(logicalDevice) {

    std::unordered_map<VkDescriptorType, uint8_t> descriptorTypeOccurances;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(uniformBuffers[0].size());

    for (uint32_t i = 0; i < layoutBindings.size(); ++i) {
        VkDescriptorType descriptorType = uniformBuffers[0][i]->getVkDescriptorType();
        layoutBindings[i].binding               = i;
        layoutBindings[i].descriptorCount       = 1;
        layoutBindings[i].descriptorType        = descriptorType;
        layoutBindings[i].pImmutableSamplers    = nullptr;
        layoutBindings[i].stageFlags            = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

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
        std::vector<VkWriteDescriptorSet> descriptorWrites(uniformBuffers[i].size());

        std::vector<VkDescriptorImageInfo> imageInfos(uniformBuffers[i].size());
        std::vector<VkDescriptorBufferInfo> bufferInfos(uniformBuffers[i].size());

        for (size_t j = 0; j < uniformBuffers[i].size(); j++) {
            descriptorWrites[j].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstSet          = _descriptorSets[i];
            descriptorWrites[j].dstBinding      = j;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorType  = uniformBuffers[i][j]->getVkDescriptorType();
            descriptorWrites[j].descriptorCount = 1;

            if (auto ptr = dynamic_cast<UniformBufferStruct*>(uniformBuffers[i][j].get())) {
                bufferInfos[j] = {
                    .buffer = ptr->getVkBuffer(),
                    .offset = 0,
                    .range  = ptr->getBufferSize(),
                };
                descriptorWrites[j].pBufferInfo = &bufferInfos[j];
            }
            else if (auto ptr = dynamic_cast<UniformBufferTexture*>(uniformBuffers[i][j].get())) {
                auto texturePtr     = ptr->getTexturePtr();
                const auto& image   = texturePtr->getImage();

                imageInfos[j] = {
                    .sampler        = texturePtr->getVkSampler(),
                    .imageView      = image.view,
                    .imageLayout    = image.layout,
                };

                descriptorWrites[j].pImageInfo = &imageInfos[j];
            }
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
