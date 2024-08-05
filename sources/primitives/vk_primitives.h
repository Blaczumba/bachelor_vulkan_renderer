#pragma once

#include "primitives.h"

#include <vulkan/vulkan.h>

#include <array>

template<typename T>
static VkVertexInputBindingDescription getBindingDescription();

template<typename T>
static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

template<>
VkVertexInputBindingDescription getBindingDescription<VertexPTNTB>() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexPTNTB);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

template<>
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions<VertexPTNTB>() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset = offsetof(VertexPTNTB, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset = offsetof(VertexPTNTB, texCoord);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].offset = offsetof(VertexPTNTB, normal);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].offset = offsetof(VertexPTNTB, tangent);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].offset = offsetof(VertexPTNTB, bitangent);

    return attributeDescriptions;
}

template<>
VkVertexInputBindingDescription getBindingDescription<VertexPTN>() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexPTN);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

template<>
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions<VertexPTN>() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset = offsetof(VertexPTN, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset = offsetof(VertexPTN, texCoord);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].offset = offsetof(VertexPTN, normal);

    return attributeDescriptions;
}

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