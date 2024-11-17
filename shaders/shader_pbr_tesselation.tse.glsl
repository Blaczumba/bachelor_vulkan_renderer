#version 450

// Define the type of tessellation - triangles and equal spacing
layout(triangles, equal_spacing, cw) in;

// Input from tessellation control shader
layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inNormal[];
layout(location = 2) in vec3 inTangent[];

// Output to fragment shader
layout(location = 0) out vec3 teTBNfragPosition;
layout(location = 1) out vec2 teFragTexCoord;
layout(location = 2) out vec4 teLightFragPosition;
layout(location = 3) out vec3 teTBNLightPos;
layout(location = 4) out vec3 teTBNViewPos;

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 viewPos;

} camera;

layout(binding = 2) uniform Light {
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
    vec3 normal = gl_TessCoord.x * inNormal[0] +
                gl_TessCoord.y * inNormal[1] +
                gl_TessCoord.z * inNormal[2];

    vec3 tangent = gl_TessCoord.x * inTangent[0] +
                gl_TessCoord.y * inTangent[1] +
                gl_TessCoord.z * inTangent[2];

    vec3 bitangent = cross(normalize(normal), normalize(tangent));
    mat3 TBNMat = transpose(mat3(tangent, bitangent, normal));

    vec4 pos = gl_TessCoord.x * gl_in[0].gl_Position +
             gl_TessCoord.y * gl_in[1].gl_Position +
             gl_TessCoord.z * gl_in[2].gl_Position;

    teLightFragPosition = BiasMat * light.projView * pos;

    gl_Position = camera.proj * camera.view * pos;

    teTBNfragPosition = TBNMat * pos.xyz;

    teTBNLightPos = TBNMat * light.pos;

    teTBNViewPos = TBNMat * camera.viewPos;

    teFragTexCoord = gl_TessCoord.x * inTexCoord[0] +
            gl_TessCoord.y * inTexCoord[1] +
            gl_TessCoord.z * inTexCoord[2];
}