#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout (location = 0) out vec3 outUVW;

void main() 
{
	// Convert cubemap coordinates into Vulkan coordinate space
	outUVW = inPos;
	// Remove translation from view matrix
	vec4 outPos = ubo.projection * mat4(mat3(ubo.view)) * vec4(inPos.xyz, 1.0);
	gl_Position = outPos.xyww;
	gl_Position.z -= 0.000001;
}
