#pragma once

#include "primitives.h"

#include <vulkan/vulkan.h>

#include <array>

template<typename T>
static VkVertexInputBindingDescription getBindingDescription();

template<typename T>
static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

template<>
VkVertexInputBindingDescription getBindingDescription<VertexPT>() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexPT);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

template<>
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions<VertexPT>() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(VertexPT, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset = offsetof(VertexPT, texCoord);

    return attributeDescriptions;
}


template<>
VkVertexInputBindingDescription getBindingDescription<VertexP>() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexP);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

template<>
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions<VertexP>() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(VertexPT, pos);

    return attributeDescriptions;
}