#version 450

// Constants for tessellation levels
const float tessLevelOuter = 1.0;  // Outer tessellation level
const float tessLevelInner = 1.0;  // Inner tessellation level

// Input from the vertex shader
layout(vertices = 3) out;  // Using triangles, so 3 control points

// Pass-through variables
layout(location = 0) in vec3 TBNfragPosition[];
layout(location = 1) in vec2 fragTexCoord[];
layout(location = 2) in vec4 lightFragPosition[];
layout(location = 3) in vec3 TBNLightPos[];
layout(location = 4) in vec3 TBNViewPos[];

// Output to tessellation evaluation shader
layout(location = 0) out vec3 tcTBNfragPosition[];
layout(location = 1) out vec2 tcFragTexCoord[];
layout(location = 2) out vec4 tcLightFragPosition[];
layout(location = 3) out vec3 tcTBNLightPos[];
layout(location = 4) out vec3 tcTBNViewPos[];

void main() {
    // Pass through the per-vertex data to the tessellation evaluation shader
    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
    tcTBNfragPosition[gl_InvocationID] = TBNfragPosition[gl_InvocationID];
    tcFragTexCoord[gl_InvocationID] = fragTexCoord[gl_InvocationID];
    tcLightFragPosition[gl_InvocationID] = lightFragPosition[gl_InvocationID];
    tcTBNLightPos[gl_InvocationID] = TBNLightPos[gl_InvocationID];
    tcTBNViewPos[gl_InvocationID] = TBNViewPos[gl_InvocationID];

    // Pass through light and view positions unchanged (same for all vertices)
    if (gl_InvocationID == 0) {  // Pass once, it's the same across the patch
        gl_TessLevelOuter[0] = tessLevelOuter;
        gl_TessLevelOuter[1] = tessLevelOuter;
        gl_TessLevelOuter[2] = tessLevelOuter;

        gl_TessLevelInner[0] = tessLevelInner;
    }
}