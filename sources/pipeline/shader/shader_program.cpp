#include "shader_program.h"

#include "descriptor_set/descriptor_set_layout.h"
#include "primitives/vk_primitives.h"
#include "primitives/primitives.h"

#include <algorithm>
#include <iterator>

ShaderProgram::ShaderProgram(const LogicalDevice& logicalDevice, std::vector<Shader>&& shaders, std::unique_ptr<DescriptorSetLayout>&& descriptorSetLayout)
    : _logicalDevice(logicalDevice), _shaders(std::move(shaders)), _descriptorSetLayout(std::move(descriptorSetLayout)) {}

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

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configurePBRProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(2);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexPTNT{});
}

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configurePBRTesselationProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(4);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr_tesselation.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr_tesselation.tsc.spv", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr_tesselation.tse.spv", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexPTNT{});
}

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configureSkyboxProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(2);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexP{});
}

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configureShadowProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(2);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexP{});
}

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configureSkyboxOffscreenProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(2);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "skybox_offscreen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexPTNT{});
}

std::unique_ptr<GraphicsShaderProgram> ShaderProgramFactory::configurePBROffscreenProgram(const LogicalDevice& logicalDevice) {
    std::vector<Shader> shaders;
    shaders.reserve(2);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders.emplace_back(logicalDevice, SHADERS_PATH "offscreen_shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>(logicalDevice);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptorSetLayout->create();
    return std::make_unique<GraphicsShaderProgram>(logicalDevice, std::move(shaders), std::move(descriptorSetLayout), VertexP{});
}