#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

template<typename VertexType, typename IndexType>
struct VertexData {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;
	std::vector<std::string> diffuseTextures;
	std::vector<std::string> normalTextures;
	std::vector<std::string> metallicRoughnessTextures;
	glm::mat4 model;
};

template<typename VertexType>
class ModelLoader {
public:
	~ModelLoader() = default;
};