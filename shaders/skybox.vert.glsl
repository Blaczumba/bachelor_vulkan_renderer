#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout (location = 0) out vec3 outUVW;

void main() {
	outUVW = inPos;
	vec4 outPos = ubo.projection * mat4(mat3(ubo.view)) * vec4(inPos.xyz, 1.0);
	gl_Position = outPos.xyww;
}
