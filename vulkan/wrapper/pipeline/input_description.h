#pragma once
#include <array>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/util/primitives.h"

template <typename T>
static constexpr VkVertexInputBindingDescription getBindingDescription();

template <typename T>
static constexpr std::array<VkVertexInputAttributeDescription, T::num_attributes>
getAttributeDescriptions();

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexPTNTB>() {
  return {.binding = 0, .stride = sizeof(VertexPTNTB), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexPTNTB::num_attributes>
getAttributeDescriptions<VertexPTNTB>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNTB, pos)      },
    VkVertexInputAttributeDescription{
                                      .location = 1,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32_SFLOAT,
                                      .offset = offsetof(VertexPTNTB, texCoord) },
    VkVertexInputAttributeDescription{
                                      .location = 2,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNTB, normal)   },
    VkVertexInputAttributeDescription{
                                      .location = 3,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNTB, tangent)  },
    VkVertexInputAttributeDescription{
                                      .location = 4,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNTB, bitangent)}
  };
}

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexPTNT>() {
  return {.binding = 0, .stride = sizeof(VertexPTNT), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexPTNT::num_attributes>
getAttributeDescriptions<VertexPTNT>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNT, pos)     },
    VkVertexInputAttributeDescription{
                                      .location = 1,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32_SFLOAT,
                                      .offset = offsetof(VertexPTNT, texCoord)},
    VkVertexInputAttributeDescription{
                                      .location = 2,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNT, normal)  },
    VkVertexInputAttributeDescription{
                                      .location = 3,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTNT, tangent) }
  };
}

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexPTN>() {
  return {.binding = 0, .stride = sizeof(VertexPTN), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexPTN::num_attributes>
getAttributeDescriptions<VertexPTN>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTN, pos)     },
    VkVertexInputAttributeDescription{
                                      .location = 1,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32_SFLOAT,
                                      .offset = offsetof(VertexPTN, texCoord)},
    VkVertexInputAttributeDescription{
                                      .location = 2,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPTN, normal)  }
  };
}

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexPT>() {
  return {.binding = 0, .stride = sizeof(VertexPT), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexPT::num_attributes>
getAttributeDescriptions<VertexPT>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPT, pos)     },
    VkVertexInputAttributeDescription{
                                      .location = 1,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32_SFLOAT,
                                      .offset = offsetof(VertexPT, texCoord)}
  };
}

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexPN>() {
  return {.binding = 0, .stride = sizeof(VertexPN), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexPN::num_attributes>
getAttributeDescriptions<VertexPN>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPN, pos)   },
    VkVertexInputAttributeDescription{
                                      .location = 1,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      .offset = offsetof(VertexPN, normal)}
  };
}

template <>
constexpr VkVertexInputBindingDescription getBindingDescription<VertexP>() {
  return {.binding = 0, .stride = sizeof(VertexP), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

template <>
constexpr std::array<VkVertexInputAttributeDescription, VertexP::num_attributes>
getAttributeDescriptions<VertexP>() {
  return {
    VkVertexInputAttributeDescription{
                                      .location = 0,
                                      .binding = 0,
                                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                                      }
  };
}

struct PushConstantsPBR {
  glm::mat4 model;
  uint32_t uniformIndex;
  uint32_t diffuse;
  uint32_t normal;
  uint32_t metallicRoughness;
  uint32_t shadow;
  uint32_t padding[3];
};

struct PushConstantsShadow {
  glm::mat4 model;
  glm::mat4 lightProjView;
};

struct PushConstantsPhongEnv {
  glm::mat4 lightProjView;
  glm::mat3x4 model;
  uint32_t envMapHandle;
};

struct PushConstantsSkybox {
  glm::mat4 proj;
  glm::mat3x4 view;
  uint32_t skyboxHandle;
};
