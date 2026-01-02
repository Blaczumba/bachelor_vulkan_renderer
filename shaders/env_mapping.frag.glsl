#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWorldNormal;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    
    // Use normal matrix (transpose of inverse) to keep normals correct if scaling exists
    outWorldNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    
    gl_Position = ubo.proj * ubo.view * worldPos;
}