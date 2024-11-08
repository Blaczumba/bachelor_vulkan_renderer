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
                vertex = {
                    .pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    }
                };

                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }
    return { vertices, indices };
}

template<>
VertexData<VertexPTN, uint32_t> TinyOBJLoaderVertex::templatedExtractor<VertexPTN>(const std::string& filePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    std::vector<VertexPTN> vertices;
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

                VertexPTN vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }
    return { vertices, indices };
}

template<>
VertexData<VertexPTNTB, uint32_t> TinyOBJLoaderVertex::templatedExtractor<VertexPTNTB>(const std::string& filePath) {
    VertexData<VertexPTN, uint32_t> baseData = templatedExtractor<VertexPTN>(filePath);

    std::vector<VertexPTNTB> vertices;
    std::vector<uint32_t> indices = std::move(baseData.indices);

    std::transform(baseData.vertices.cbegin(), baseData.vertices.cend(), std::back_inserter(vertices), [](const VertexPTN& vertex) {
        return VertexPTNTB{
            .pos        = vertex.pos,
            .texCoord   = vertex.texCoord,
            .normal     = glm::normalize(vertex.normal)
        };
        }
    );

    // Calculate tangents and bitangents
    for (size_t i = 0; i < indices.size(); i += 3) {
        VertexPTNTB& v0 = vertices[indices[i + 0]];
        VertexPTNTB& v1 = vertices[indices[i + 1]];
        VertexPTNTB& v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.pos - v0.pos;
        glm::vec3 edge2 = v2.pos - v0.pos;
        glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent = glm::normalize(f * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
        glm::vec3 bitangent = glm::normalize(f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2));

        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;

        v0.bitangent = bitangent;
        v1.bitangent = bitangent;
        v2.bitangent = bitangent;
    }

    return { vertices, indices };
}
