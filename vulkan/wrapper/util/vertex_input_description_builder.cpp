#include "vertex_input_description_builder.h"

VertexInputDescriptionBuilder& VertexInputDescriptionBuilder::finishBinding(
    VkVertexInputRate vertexInputRate) {
  _VkVertexInputBindingDescription.emplace_back(_currentBinding, _currentOffset, vertexInputRate);
  _currentBinding = _currentOffset = 0;
  return *this;
}

std::tuple<std::span<const VkVertexInputBindingDescription>,
           std::span<const VkVertexInputAttributeDescription>>
VertexInputDescriptionBuilder::getDescription() const {
  return std::make_tuple(
      std::span(_VkVertexInputBindingDescription), std::span(_vkVertexInputAttributeDescription));
}
