#pragma once

#include "pipeline.h"
#include "render_pass/render_pass.h"
#include "logical_device/logical_device.h"
#include "primitives/vk_primitives.h"
#include "memory_objects/uniform_buffer/push_constants.h"
#include "shader/shader.h"
#include "shader/shader_program.h"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <algorithm>
#include <string>
#include <stdexcept>

struct GraphicsPipelineParameters {
    VkCullModeFlags cullMode                    = VK_CULL_MODE_BACK_BIT;
    VkSampleCountFlagBits msaaSamples           = VK_SAMPLE_COUNT_1_BIT;
    std::optional<uint32_t> patchControlPoints  = std::nullopt;
    float depthBiasConstantFactor               = 0.0f;
    float depthBiasSlopeFactor                  = 0.0f;
};

class GraphicsPipeline : public Pipeline {
    GraphicsPipelineParameters _parameters;

    const Renderpass& _renderpass;
    GraphicsShaderProgram* _shaderProgram = nullptr;

public:
	GraphicsPipeline(const Renderpass& renderpass);
	~GraphicsPipeline();
    void create();
    void cleanup();

    void setPipelineParameters(const GraphicsPipelineParameters& parameters);
    void setShaderProgram(GraphicsShaderProgram* shaderProgram);
};

