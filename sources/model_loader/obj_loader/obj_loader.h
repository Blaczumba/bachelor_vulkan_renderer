#pragma once

#include "model_loader/model_loader.h"
#include "primitives/primitives.h"

#include <tinyobjloader/tiny_obj_loader.h>

#include <unordered_map>
#include <stdexcept>

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


class TinyOBJLoaderVertex : public ModelLoader<VertexPT> {
    template<typename IndexType>
    VertexData<VertexPT, IndexType> templatedExtractor(const std::string&);
public:
	TinyOBJLoaderVertex() = default;
    template<typename IndexType>
	VertexData<VertexPT, IndexType> extract(const std::string& filePath);
};

template<typename IndexType> 
VertexData<VertexPT, IndexType> TinyOBJLoaderVertex::templatedExtractor(const std::string& filePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    std::vector<VertexPT> vertices;
    std::vector<IndexType> indices;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filePath.data())) {
        throw std::runtime_error(warning + error);
    }

    std::unordered_map<Indices, int, Indices::Hash> mp;
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Indices idx = Indices{ index.vertex_index, index.normal_index, index.texcoord_index };
            if (auto ptr = mp.find(idx); ptr != mp.cend()) {
                indices.push_back(ptr->second);
            }
            else {
                mp.insert({ idx, static_cast<IndexType>(vertices.size()) });

                VertexPT vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                indices.push_back(static_cast<IndexType>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }
    return { vertices, indices };
}
