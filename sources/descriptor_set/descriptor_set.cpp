#include "descriptor_set.h"

#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<DescriptorSetLayoutElement>& layoutElements)
    : _logicalDevice(logicalDevice), _layoutElements(layoutElements) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(_layoutElements.size());

	for (uint32_t bindingIndex = 0; bindingIndex < _layoutElements.size(); ++bindingIndex) {
        bindings.emplace_back(
            VkDescriptorSetLayoutBinding{
                    .binding = bindingIndex,
                    .descriptorType = _layoutElements[bindingIndex].type,
                    .descriptorCount = 1,
                    .stageFlags = _layoutElements[bindingIndex].stage,
                    .pImmutableSamplers = nullptr
            }
        );
	}

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_logicalDevice->getVkDevice(), &layoutInfo, nullptr, &_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(_logicalDevice->getVkDevice(), _layout, nullptr);
}

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout)
	: _layout(layout) {

}
