#pragma once

#include "model_loader/model_loader.h"
#include "primitives/primitives.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

glm::mat4 GetNodeTransform(const tinygltf::Node& node) {
    glm::mat4 mat(1.0f);

    if (node.matrix.size() == 16) {
        mat = glm::make_mat4(node.matrix.data());
    }
    else {
        if (node.translation.size() == 3) {
            mat = glm::translate(mat, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if (node.rotation.size() == 4) {
            glm::quat quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            mat *= glm::mat4_cast(quat);
        }
        if (node.scale.size() == 3) {
            mat = glm::scale(mat, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
    }

    return mat;
}

template<typename VertexType, typename IndexType>
void ProcessNode(const tinygltf::Model& model, const tinygltf::Node& node, glm::mat4 parentTransform, std::vector<VertexData<VertexType, IndexType>>& vertexDataList) {
    glm::mat4 currentTransform = parentTransform * GetNodeTransform(node);

    if (node.mesh < 0) {
        for (const auto& childIndex : node.children) {
            ProcessNode<VertexType, IndexType>(model, model.nodes[childIndex], currentTransform, vertexDataList);
        }
        return;
    }

    const auto& mesh = model.meshes[node.mesh];
    VertexData<VertexType, IndexType> vertexData;
    vertexData.model = currentTransform;

    for (const auto& primitive : mesh.primitives) {
        const auto& attributes = primitive.attributes;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;

        if constexpr (VertexTraits<VertexType>::hasPosition) {
            if (attributes.find("POSITION") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("POSITION")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                positions.reserve(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i) {
                    positions.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }
        }

        if constexpr (VertexTraits<VertexType>::hasTexCoord) {
            if (attributes.find("TEXCOORD_0") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                texCoords.reserve(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i) {
                    texCoords.emplace_back(data[i * 2], data[i * 2 + 1]);
                }
            }
        }

        if constexpr (VertexTraits<VertexType>::hasNormal) {
            if (attributes.find("NORMAL") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                normals.reserve(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i) {
                    normals.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }
        }

        //if constexpr (VertexTraits<VertexType>::hasTangent) {
        //    if (attributes.find("TANGENT") != attributes.end()) {
        //        const tinygltf::Accessor& accessor = model.accessors[attributes.at("TANGENT")];
        //        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        //        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        //        const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        //        tangents.reserve(accessor.count);
        //        for (size_t i = 0; i < accessor.count; ++i) {
        //            tangents.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        //        }
        //    }
        //}

        // Combine vertex attributes into VertexType
        vertexData.vertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            VertexType vertex{};
            if constexpr (VertexTraits<VertexType>::hasPosition) vertex.pos = positions[i];
            if constexpr (VertexTraits<VertexType>::hasTexCoord) 
                if (i < texCoords.size()) vertex.texCoord = texCoords[i];
            if constexpr (VertexTraits<VertexType>::hasNormal)
                if (i < normals.size()) vertex.normal = normals[i];
            if constexpr (VertexTraits<VertexType>::hasTangent)
                if (i < tangents.size()) vertex.tangent = tangents[i];
            if constexpr (VertexTraits<VertexType>::hasBitangent)
                if (i < bitangents.size()) vertex.bitangent = bitangents[i];
            vertexData.vertices.push_back(vertex);
        }

        // Load indices
        if (primitive.indices >= 0) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            vertexData.indices.reserve(accessor.count);
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const uint16_t* data = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    vertexData.indices.push_back(static_cast<IndexType>(data[i]));
                }
            }
            else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const uint32_t* data = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    vertexData.indices.push_back(static_cast<IndexType>(data[i]));
                }
            }
        }

        // Load textures
        if (primitive.material >= 0) {
            const tinygltf::Material& material = model.materials[primitive.material];

            if (material.values.find("baseColorTexture") != material.values.end()) {
                const tinygltf::Texture& texture = model.textures[material.values.at("baseColorTexture").TextureIndex()];
                const tinygltf::Image& image = model.images[texture.source];
                vertexData.diffuseTextures.push_back(image.uri);
            }
            if (material.values.find("metallicRoughnessTexture") != material.values.end()) {
                const tinygltf::Texture& texture = model.textures[material.values.at("metallicRoughnessTexture").TextureIndex()];
                const tinygltf::Image& image = model.images[texture.source];
                vertexData.metallicRoughnessTextures.push_back(image.uri);
            }
            if (material.additionalValues.find("normalTexture") != material.additionalValues.end()) {
                const tinygltf::Texture& texture = model.textures[material.additionalValues.at("normalTexture").TextureIndex()];
                const tinygltf::Image& image = model.images[texture.source];
                vertexData.normalTextures.push_back(image.uri);
            }
        }
    }

    vertexDataList.push_back(vertexData);

    // Calculate tangents and bitangents (only if the vertex type has them)
    if constexpr (VertexTraits<VertexType>::hasTangent || VertexTraits<VertexType>::hasBitangent) {
        for (size_t i = 0; i < vertexDataList.back().indices.size(); i += 3) {
            VertexType& v0 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 0]];
            VertexType& v1 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 1]];
            VertexType& v2 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 2]];

            glm::vec3 edge1 = v1.pos - v0.pos;
            glm::vec3 edge2 = v2.pos - v0.pos;
            glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
            glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

            glm::vec3 tangent = glm::normalize(deltaUV2.y * edge1 - deltaUV1.y * edge2);

            // float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);


            if constexpr (VertexTraits<VertexType>::hasTangent) {
                glm::vec3 tangent = glm::normalize(deltaUV2.y * edge1 - deltaUV1.y * edge2); // f scale
                v0.tangent = tangent;
                v1.tangent = tangent;
                v2.tangent = tangent;
            }

            if constexpr (VertexTraits<VertexType>::hasBitangent) {
                glm::vec3 bitangent = glm::normalize(-deltaUV2.x * edge1 + deltaUV1.x * edge2); // f scale
                v0.bitangent = bitangent;
                v1.bitangent = bitangent;
                v2.bitangent = bitangent;
            }
        }
    }

    for (const auto& childIndex : node.children) {
        ProcessNode<VertexType, IndexType>(model, model.nodes[childIndex], currentTransform, vertexDataList);
    }
}

template<typename VertexType, typename IndexType>
std::vector<VertexData<VertexType, IndexType>> LoadGLTF(const std::string& filePath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret;

    if (filePath.find(".glb") != std::string::npos || filePath.find(".bin") != std::string::npos)
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
    else if (filePath.find(".gltf") != std::string::npos)
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
    else
        ret = false;

    if (!ret) {
        throw std::runtime_error("Failed to load GLTF file: " + filePath + "\n" + err);
    }

    std::vector<VertexData<VertexType, IndexType>> vertexDataList;

    for (const auto& scene : model.scenes) {
        for (const auto& nodeIndex : scene.nodes) {
            const tinygltf::Node& node = model.nodes[nodeIndex];
            ProcessNode<VertexType, IndexType>(model, node, glm::mat4(1.0f), vertexDataList);
        }
    }

    return vertexDataList;
}