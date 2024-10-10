#version 450

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 viewPos;

} camera;

layout(binding = 2) uniform Light {
    mat4 projView;

    vec3 pos;

} light;

layout(binding = 3) uniform ObjectUniform {
    mat4 model;

} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 lightFragPosition;

const mat4 BiasMat = mat4(
	0.5, 0, 0, 0,
	0, 0.5, 0, 0,
	0, 0, 1.0, 0,
	0.5, 0.5, 0.0, 1.0
);


void main() {
    gl_Position = object.model *  vec4(inPosition, 1.0);
    fragPosition = gl_Position.xyz;
    lightFragPosition = BiasMat * light.projView * gl_Position;

    gl_Position = camera.proj * camera.view * gl_Position;
    
    fragTexCoord = inTexCoord;
    fragNormal = vec3(inverse(transpose(object.model)) * vec4(inNormal, 1.0));
}