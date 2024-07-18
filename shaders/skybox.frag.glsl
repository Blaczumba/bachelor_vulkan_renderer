#version 450

layout (binding = 1) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outColor;

void main() 
{
	outFragColor = texture(samplerCubeMap, inUVW);
	outColor = outFragColor;
}