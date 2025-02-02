#version 450

// Define the type of tessellation - triangles and equal spacing
layout(triangles, equal_spacing, cw) in;

// Input from tessellation control shader
layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inNormal[];

// Output to fragment shader
layout(location = 0) out vec3 tefragPosition;
layout(location = 1) out vec2 teFragTexCoord;
layout(location = 2) out vec4 teLightFragPosition;
layout(location = 3) out vec3 teNormal;

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 viewPos;

} camera;

layout(binding = 1) uniform Light {
    mat4 projView;

    vec3 pos;

} light;

const mat4 BiasMat = mat4(
	0.5, 0, 0, 0,
	0, 0.5, 0, 0,
	0, 0, 1.0, 0,
	0.5, 0.5, 0.0, 1.0
);

void main() {

    vec4 pos = gl_TessCoord.x * gl_in[0].gl_Position +
             gl_TessCoord.y * gl_in[1].gl_Position +
             gl_TessCoord.z * gl_in[2].gl_Position;

    gl_Position = camera.proj * camera.view * pos;

    tefragPosition = pos.xyz;
    teFragTexCoord = gl_TessCoord.x * inTexCoord[0] +
            gl_TessCoord.y * inTexCoord[1] +
            gl_TessCoord.z * inTexCoord[2];
    teLightFragPosition = BiasMat * light.projView * pos;
    teNormal = gl_TessCoord.x * inNormal[0] +
                gl_TessCoord.y * inNormal[1] +
                gl_TessCoord.z * inNormal[2];
}