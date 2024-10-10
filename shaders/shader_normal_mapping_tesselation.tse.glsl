#version 450

// Define the type of tessellation - triangles and equal spacing
layout(triangles, equal_spacing, cw) in;

// Input from tessellation control shader
layout(location = 0) in vec3 tcTBNfragPosition[];
layout(location = 1) in vec2 tcFragTexCoord[];
layout(location = 2) in vec4 tcLightFragPosition[];
layout(location = 3) in vec3 tcTBNLightPos[];
layout(location = 4) in vec3 tcTBNViewPos[];
layout(location = 5) in vec4 tcPosition[];

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

void main() {
    // Interpolate per-vertex data using barycentric coordinates
    vec4 pos =  gl_TessCoord.x * tcPosition[0] +
                gl_TessCoord.y * tcPosition[1] +
                gl_TessCoord.z * tcPosition[2];

    vec3 tessPos = gl_TessCoord.x * tcTBNfragPosition[0] +
                   gl_TessCoord.y * tcTBNfragPosition[1] +
                   gl_TessCoord.z * tcTBNfragPosition[2];

    vec2 tessTexCoord = gl_TessCoord.x * tcFragTexCoord[0] +
                        gl_TessCoord.y * tcFragTexCoord[1] +
                        gl_TessCoord.z * tcFragTexCoord[2];

    vec4 tessLightFragPosition = gl_TessCoord.x * tcLightFragPosition[0] +
                                 gl_TessCoord.y * tcLightFragPosition[1] +
                                 gl_TessCoord.z * tcLightFragPosition[2];

    vec3 tessTBNLightPos = gl_TessCoord.x * tcTBNLightPos[0] +
                           gl_TessCoord.y * tcTBNLightPos[1] +
                           gl_TessCoord.z * tcTBNLightPos[2];

    vec3 tessTBNViewPos = gl_TessCoord.x * tcTBNViewPos[0] +
                          gl_TessCoord.y * tcTBNViewPos[1] +
                          gl_TessCoord.z * tcTBNViewPos[2];

    gl_Position = camera.proj * camera.view * vec4(pos.xyz, 1.0); 

    // Pass the interpolated and transformed data to the fragment shader
    teTBNfragPosition = tessPos;
    teFragTexCoord = tessTexCoord;
    teLightFragPosition = tessLightFragPosition;
    teTBNLightPos = tessTBNLightPos;
    teTBNViewPos = tessTBNViewPos;
}