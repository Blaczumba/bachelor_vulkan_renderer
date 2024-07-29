#version 450

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 viewPos;

} camera;

layout(binding = 3) uniform ObjectUniform {
    mat4 model;

} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

void main() {
    gl_Position = object.model *  vec4(inPosition, 1.0);
    fragPosition = gl_Position.xyz;

    gl_Position = camera.proj * camera.view * gl_Position;
    
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
}