#version 450

#include "bindless.glsl"
#include "32bit_push_constants.glsl"

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;

RegisterUniform(Light, { \
    mat4 projView; \
    vec3 pos; \
});

// Push constants definitions:
#define modelMat pushConstants.model
#define environmentMapHandle pushConstants.handles[0]
#define lightBufferHandle pushConstants.handles[1]

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 pos;
    vec3 viewDir;
} camera;

layout (location = 0) out vec4 outFragColor;

const vec3 color = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 N = normalize(inNormal);
    vec3 V = normalize(camera.pos - inWorldPos);
    
    vec3 L = normalize(GetResource(Light, lightBufferHandle).pos - inWorldPos);
    vec3 H = normalize(L + V); // Halfway vector for Blinn-Phong

    // 2. Blinn-Phong Lighting Components
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    
    vec3 diffuse = diff * color;
    vec3 specular = spec * color;

    // 3. Environment Mapping (Reflection)
    vec3 R = reflect(-V, N);
    vec3 envColor = texture(uGlobalTexturesCube[nonuniformEXT(environmentMapHandle)], R).rgb;

    // 4. Combine results
    vec3 finalColor = (envColor * 0.1) + envColor*(diffuse * 0.5 + specular);

    outFragColor = vec4(finalColor, 1.0);
}
