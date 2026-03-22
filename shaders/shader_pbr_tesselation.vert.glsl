#version 450

#include "32bit_push_constants.glsl"

// Push constants definitions:
#define modelMat pushConstants.model

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 viewDir;

} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;

void main() {
    gl_Position = modelMat * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
    mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
    outNormal = normalize(normalMatrix * inNormal);
    outTangent = normalize(normalMatrix * inTangent);
}