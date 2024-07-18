#pragma once

#include <vector>
#include <string>

template<typename VertexType, typename IndexType>
struct VertexData {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;
};

template<typename VertexType>
class ModelLoader {
public:
	~ModelLoader() = default;
};