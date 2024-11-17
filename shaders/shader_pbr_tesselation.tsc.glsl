#version 450

// Constants for tessellation levels
const float tessLevelOuter = 1.0;  // Outer tessellation level
const float tessLevelInner = 2.0;  // Inner tessellation level

layout(vertices = 3) out;  // Using triangles, so 3 control points

layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inNormal[];
layout(location = 2) in vec3 inTangent[];

// Output to tessellation evaluation shader
layout(location = 0) out vec2 outTexCoord[];
layout(location = 1) out vec3 outNormal[];
layout(location = 2) out vec3 outTangent[];


void main() {
    // Pass through the per-vertex data to the tessellation evaluation shader
    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
    outTexCoord[gl_InvocationID] = inTexCoord[gl_InvocationID];
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outTangent[gl_InvocationID] = inTangent[gl_InvocationID];

    if (gl_InvocationID == 0) {  // Pass once, it's the same across the patch
        gl_TessLevelOuter[0] = tessLevelOuter;
        gl_TessLevelOuter[1] = tessLevelOuter;
        gl_TessLevelOuter[2] = tessLevelOuter;

        gl_TessLevelInner[0] = tessLevelInner;
    }
}