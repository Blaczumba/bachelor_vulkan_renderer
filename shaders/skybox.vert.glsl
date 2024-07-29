#version 450

layout (location = 0) in vec3 inPos;

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 pos;

} camera;

layout (location = 0) out vec3 outUVW;

void main() {
	outUVW = inPos;
	vec4 outPos = camera.proj * mat4(mat3(camera.view)) * vec4(inPos.xyz, 1.0);
	gl_Position = outPos.xyww;
}
