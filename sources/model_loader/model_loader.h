#pragma once

#include <vector>
#include <string>

template<typename VertexType, typename IndexType>
struct VertexData {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;
};

template<typename VertexType, typename IndexType>
class ModelLoader {
public:
	virtual ~ModelLoader() = default;

	virtual VertexData<VertexType, IndexType> extract(const std::string&) { return{}; };
};