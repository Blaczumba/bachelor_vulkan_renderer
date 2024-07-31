#version 450

layout(binding = 0) uniform Light {
    mat4 projView;

    vec3 pos;

} light;

layout(binding = 1) uniform ObjectUniform {
    mat4 model;

} object;

layout (location = 0) in vec3 inPosition;

void main() {
    gl_Position = light.projView * object.model * vec4(inPosition, 1.0);
} 
