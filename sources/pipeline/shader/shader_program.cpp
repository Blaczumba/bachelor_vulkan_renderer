#include "shader_program.h"

#include "descriptor_set/descriptor_set_layout.h"
#include "primitives/vk_primitives.h"

#include <algorithm>
#include <iterator>

ShaderProgram::ShaderProgram(const LogicalDevice& logicalDevice) : _logicalDevice(logicalDevice) {}

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

PBRShaderProgram::PBRShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexPTNT>(logicalDevice) {
	_shaders.reserve(2);
	_shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	_shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	_descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();
}

PBRTesselationShaderProgram::PBRTesselationShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexPTNT>(logicalDevice) {
    _shaders.reserve(4);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr_tesselation.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr_tesselation.tsc.spv", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr_tesselation.tse.spv", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();
}

SkyboxShaderProgram::SkyboxShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexP>(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();
}

ShadowShaderProgram::ShadowShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexP>(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->create();
}

PBRShaderOffscreenProgram::PBRShaderOffscreenProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexPTNT>(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "offscreen_shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();
}

SkyboxOffscreenShaderProgram::SkyboxOffscreenShaderProgram(const LogicalDevice& logicalDevice) : GraphicsShaderProgram<VertexP>(logicalDevice) {
    _shaders.reserve(2);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    _shaders.emplace_back(_logicalDevice, SHADERS_PATH "skybox_offscreen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    _descriptorSetLayout = std::make_unique<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();
}