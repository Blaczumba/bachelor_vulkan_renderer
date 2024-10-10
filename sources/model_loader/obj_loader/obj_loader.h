#pragma once

#include "model_loader/model_loader.h"
#include "primitives/primitives.h"

#include <tinyobjloader/tiny_obj_loader.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_map>

struct Indices {
    int a;
    int b;
    int c;

    struct Hash {
        std::size_t operator()(const Indices& triple) const {
            std::size_t hash = 0;
            hash ^= std::hash<int>{}(triple.a) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int>{}(triple.b) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int>{}(triple.c) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    bool operator==(const Indices& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};


class TinyOBJLoaderVertex {
    template<typename VertexType>
    static VertexData<VertexType, uint32_t> templatedExtractor(const std::string&);
public:
	TinyOBJLoaderVertex() = default;

    template<typename VertexType, typename IndexType>
	static VertexData<VertexType, IndexType> extract(const std::string& filePath);
};

template<typename VertexType, typename IndexType>
VertexData<VertexType, IndexType> TinyOBJLoaderVertex::extract(const std::string& filePath) {
    VertexData<VertexType, uint32_t> data =  templatedExtractor<VertexType>(filePath);
    VertexData<VertexType, IndexType> output;
    output.vertices = std::move(data.vertices);
    std::transform(data.indices.cbegin(), data.indices.cend(), std::back_inserter(output.indices), [](uint32_t index) { return static_cast<IndexType>(index); });
    return output;
}

