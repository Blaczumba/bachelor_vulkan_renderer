#include "shader.h"
#include "shader_utils.h"

#include "logical_device/logical_device.h"

#include <stdexcept>

Shader::Shader(const LogicalDevice& logicalDevice, const std::string& shaderPath, const VkShaderStageFlagBits shaderStage) : _logicalDevice(logicalDevice), _shaderStage(shaderStage), _name("main") {
    const auto shaderCode = readFile(shaderPath);

    const VkDevice device = _logicalDevice.getVkDevice();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = _shaderStage;
    vertShaderStageInfo.module = _shaderModule;
    vertShaderStageInfo.pName = _name.data();
}

Shader::~Shader() {
    vkDestroyShaderModule(_logicalDevice.getVkDevice(), _shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::getVkPipelineStageCreateInfo() const {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = _shaderStage;
    vertShaderStageInfo.module = _shaderModule;
    vertShaderStageInfo.pName = _name.data();

    return vertShaderStageInfo;
}

VkShaderStageFlagBits Shader::getVkShaderStageFlagBits() const {
    return _shaderStage;
}
