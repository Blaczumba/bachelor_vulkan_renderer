#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "bindless.glsl"

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;

RegisterUniform(Light, { \
    mat4 projView; \
    vec3 pos; \
});

layout(push_constant) uniform Constants {
    mat4 model;
    uint16_t environmentHandle;
    uint16_t light;
    uint16_t padding[30];
} pushConstants;

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

layout (location = 0) out vec4 outFragColor;

const vec3 color = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 N = normalize(inNormal);
    vec3 V = normalize(camera.pos - inWorldPos);
    
    vec3 L = normalize(GetResource(Light, pushConstants.light).pos - inWorldPos);
    vec3 H = normalize(L + V); // Halfway vector for Blinn-Phong

    // 2. Blinn-Phong Lighting Components
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    
    vec3 diffuse = diff * color;
    vec3 specular = spec * color;

    // 3. Environment Mapping (Reflection)
    vec3 R = reflect(-V, N);
    vec3 envColor = texture(uGlobalTexturesCube[nonuniformEXT(pushConstants.environmentHandle)], R).rgb;

    // 4. Combine results
    vec3 finalColor = (envColor * 0.1) + envColor*(diffuse * 0.5 + specular);

    outFragColor = vec4(finalColor, 1.0);
}
