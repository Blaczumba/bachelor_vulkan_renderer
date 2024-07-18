#include "obj_loader.h"

template<>
VertexData<VertexPT, uint32_t> TinyOBJLoaderVertex::templatedExtractor<VertexPT>(const std::string& filePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    std::vector<VertexPT> vertices;
    std::vector<uint32_t> indices;

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
                mp.insert({ idx, static_cast<uint32_t>(vertices.size()) });

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

                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }
    return { vertices, indices };
}

template<>
VertexData<VertexP, uint32_t> TinyOBJLoaderVertex::templatedExtractor<VertexP>(const std::string& filePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    std::vector<VertexP> vertices;
    std::vector<uint32_t> indices;

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
                mp.insert({ idx, static_cast<uint32_t>(vertices.size()) });

                VertexP vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }
    return { vertices, indices };
}