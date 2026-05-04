#include "shader.h"

#include <span>
#include <string_view>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/util/check.h"

Shader::Shader(VkShaderModule shaderModule, const LogicalDevice& logicalDevice,
               VkShaderStageFlagBits shaderStage) noexcept
  : _shaderModule(shaderModule), _logicalDevice(&logicalDevice), _shaderStage(shaderStage) {}

Shader Shader::create(const LogicalDevice& logicalDevice, std::span<const std::byte> shaderData,
                      VkShaderStageFlagBits shaderStage) {
  const VkShaderModuleCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = shaderData.size(),
    .pCode = reinterpret_cast<const uint32_t*>(shaderData.data())};

  VkShaderModule shaderModule;
  CHECK_VKCMD(
      vkCreateShaderModule(logicalDevice.getVkDevice(), &createInfo, nullptr, &shaderModule),
      "Failed to create VkShaderModule.");
  return Shader(shaderModule, logicalDevice, shaderStage);
}

Shader::Shader(Shader&& other) noexcept
  : _shaderModule(std::exchange(other._shaderModule, VK_NULL_HANDLE)),
    _logicalDevice(other._logicalDevice), _shaderStage(other._shaderStage) {}

void Shader::destroy() {
  if (_shaderModule != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([shaderModule = _shaderModule](DestroyerContext context) {
      vkDestroyShaderModule(context.device, shaderModule, context.allocationCallbacks);
    });
  }
}

Shader& Shader::operator=(Shader&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  destroy();

  _shaderModule = std::exchange(other._shaderModule, VK_NULL_HANDLE);
  _logicalDevice = other._logicalDevice;
  _shaderStage = other._shaderStage;
  return *this;
}

Shader::~Shader() {
  destroy();
}

VkPipelineShaderStageCreateInfo Shader::getVkPipelineStageCreateInfo() const noexcept {
  static constexpr std::string_view pname = "main";
  return VkPipelineShaderStageCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = _shaderStage,
    .module = _shaderModule,
    .pName = pname.data()};
}

VkShaderStageFlagBits Shader::getVkShaderStageFlagBits() const noexcept {
  return _shaderStage;
}
