#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout(push_constant) uniform Constants {
    mat4 model;
    uint environmentHandle;
    uint light;
    uint padding[14];
} pushConstants;

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} camera;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;

void main() {
    outNormal = normalize(mat3(transpose(inverse(pushConstants.model))) * inNormal);
    
    vec4 pos = pushConstants.model * vec4(inPos, 1.0);
    outWorldPos = vec3(pos);

    gl_Position = camera.proj * camera.view * pos;
}