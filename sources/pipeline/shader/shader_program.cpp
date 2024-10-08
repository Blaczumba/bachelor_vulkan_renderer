#include "shader_program.h"
#include "descriptor_set/descriptor_set_layout.h"
#include "primitives/vk_primitives.h"

#include <algorithm>
#include <iterator>

ShaderProgram::ShaderProgram(const LogicalDevice& logicalDevice) : _logicalDevice(logicalDevice) {

}

const DescriptorSetLayout& ShaderProgram::getDescriptorSetLayout() const {
    return *_descriptorSetLayout;
}

const PushConstants& ShaderProgram::getPushConstants() const {
    return _pushConstants;
}

std::vector<VkPipelineShaderStageCreateInfo> ShaderProgram::getVkPipelineShaderStageCreateInfos() const {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(_shaders.size());
    std::transform(_shaders.cbegin(), _shaders.cend(), std::back_inserter(shaderStages), [](const Shader& shader) { return shader.getVkPipelineStageCreateInfo(); });
    return shaderStages;
}

GraphicsShaderProgram::GraphicsShaderProgram(const LogicalDevice& logicalDevice) : ShaderProgram(logicalDevice) {

}

const VkPipelineVertexInputStateCreateInfo& GraphicsShaderProgram::getVkPipelineVertexInputStateCreateInfo() const {
    return _vertexInputInfo;
}

PBRShaderProgram::PBRShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram(logicalDevice) {
	_shaders.reserve(4);
	_shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_normal_mapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    // _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_normal_mapping_tesselation.tsc.spv", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    // _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_normal_mapping_tesselation.tse.spv", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
	_shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_normal_mapping.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	_descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();

    static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexPTNT>();
    static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexPTNT>();

    _vertexInputInfo = VkPipelineVertexInputStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
}

SkyboxShaderProgram::SkyboxShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();

    static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexP>();
    static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexP>();

    _vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
}

ShadowShaderProgram::ShadowShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->create();

    static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexP>();
    static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexP>();

    _vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
}

PBRShaderOffscreenProgram::PBRShaderOffscreenProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_normal_mapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "offscreen_shader_normal_mapping.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();

    static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexPTNT>();
    static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexPTNT>();

    _vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
}

SkyboxOffscreenShaderProgram::SkyboxOffscreenShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox_offscreen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();

    static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexP>();
    static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexP>();

    _vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
}