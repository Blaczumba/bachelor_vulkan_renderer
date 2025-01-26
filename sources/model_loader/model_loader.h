#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <vector>

enum class IndexTypeT : uint8_t {
	NONE = 0,
	UINT8 = 1,
	UINT16 = 2,
	UINT32 = 4
};

template<typename VertexType, typename IndexType>
struct VertexData {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;
	std::vector<std::string> diffuseTextures;
	std::vector<std::string> normalTextures;
	std::vector<std::string> metallicRoughnessTextures;
	glm::mat4 model;

	std::unique_ptr<uint8_t[]> indicesS;
	uint32_t indicesCount;
	IndexTypeT indexType;
};

template<typename VertexType>
class ModelLoader {
public:
	~ModelLoader() = default;
};