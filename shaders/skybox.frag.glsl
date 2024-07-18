#version 450

layout (binding = 1) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outFragColor1;

void main() 
{
	outFragColor = texture(samplerCubeMap, inUVW);
	outFragColor1 = outFragColor;
	outFragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}