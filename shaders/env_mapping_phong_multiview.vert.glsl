#version 450

#extension GL_EXT_multiview : enable

#include "32bit_push_constants.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

// Push constants definitions:
#define modelMat pushConstants.model

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 viewDir;
} camera[2];

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;

void main() {
    outNormal = normalize(mat3(transpose(inverse(modelMat))) * inNormal);
    
    vec4 pos = modelMat * vec4(inPos, 1.0);
    outWorldPos = vec3(pos);

    gl_Position = camera[gl_ViewIndex].proj * camera[gl_ViewIndex].view * pos;
}