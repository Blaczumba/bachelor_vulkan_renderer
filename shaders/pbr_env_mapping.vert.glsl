#version 450

#extension GL_EXT_multiview : enable

#include "bindless.glsl"
#include "32bit_push_constants.glsl"

RegisterUniform(UniformData, { \
    mat4 projView[6]; \
    vec3 viewPos; \
    mat4 lightProjView; \
    vec3 lightPos; \
});

// Push constants definitions:
#define modelMat pushConstants.model
#define uniformBufferIndex pushConstants.handles[0]
#define diffuseMapHandle pushConstants.handles[1]
#define normalMapHandle pushConstants.handles[2]
#define metallicRoughnessMapHandle pushConstants.handles[3]
#define shadowMapHandle pushConstants.handles[4]

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
// layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec3 TBNfragPosition;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 lightFragPosition;

layout(location = 3) out vec3 TBNLightPos;
layout(location = 4) out vec3 TBNViewPos;

const mat4 BiasMat = mat4(
	0.5, 0, 0, 0,
	0, 0.5, 0, 0,
	0, 0, 1.0, 0,
	0.5, 0.5, 0.0, 1.0
);

void main() {
    mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
    vec3 normal = normalize(normalMatrix * inNormal);
    // vec3 tangent = normalize(normalMatrix * inTangent);
    // vec3 bitangent = normalize(normalMatrix * inBitangent);
    vec3 tangent = normalize(inTangent - dot(inTangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);
    mat3 TBNMat = transpose(mat3(tangent, bitangent, normal));

    gl_Position = modelMat * vec4(inPosition, 1.0);
    TBNfragPosition = TBNMat * gl_Position.xyz;
    TBNViewPos = TBNMat * GetResource(UniformData, uniformBufferIndex).viewPos;
    TBNLightPos = TBNMat * GetResource(UniformData, uniformBufferIndex).lightPos;
    lightFragPosition = BiasMat * GetResource(UniformData, uniformBufferIndex).lightProjView * gl_Position;

    gl_Position = GetResource(UniformData, uniformBufferIndex).projView[gl_ViewIndex] * gl_Position;
    
    fragTexCoord = inTexCoord;
}