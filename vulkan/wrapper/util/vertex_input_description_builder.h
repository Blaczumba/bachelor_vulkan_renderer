#pragma once

#include <glm/glm.hpp>
#include <span>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.h>

class VertexInputDescriptionBuilder {
public:
  VertexInputDescriptionBuilder() noexcept = default;

  ~VertexInputDescriptionBuilder() = default;

  template <typename T>
  VertexInputDescriptionBuilder& addVertexAttributeDescription();

  VertexInputDescriptionBuilder& finishBinding(VkVertexInputRate vertexInputRate);

  std::tuple<std::span<const VkVertexInputBindingDescription>,
             std::span<const VkVertexInputAttributeDescription>>
  getDescription() const noexcept;

private:
  std::vector<VkVertexInputBindingDescription> _VkVertexInputBindingDescription;
  std::vector<VkVertexInputAttributeDescription> _vkVertexInputAttributeDescription;

  uint32_t _currentBinding = 0;
  uint32_t _currentOffset = 0;
  uint32_t _currentLocation = 0;
};

namespace {

template <typename T>
struct VertexDescriptionTraits;

template <>
struct VertexDescriptionTraits<glm::vec3> {
  static constexpr VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
};

template <>
struct VertexDescriptionTraits<glm::vec2> {
  static constexpr VkFormat format = VK_FORMAT_R32G32_SFLOAT;
};

template <>
struct VertexDescriptionTraits<float> {
  static constexpr VkFormat format = VK_FORMAT_R32_SFLOAT;
};

}  // namespace

template <typename T>
VertexInputDescriptionBuilder& VertexInputDescriptionBuilder::addVertexAttributeDescription() {
  _vkVertexInputAttributeDescription.emplace_back(
      _currentLocation++, _currentBinding, VertexDescriptionTraits<T>::format, _currentOffset);
  _currentOffset += sizeof(T);
  return *this;
}
