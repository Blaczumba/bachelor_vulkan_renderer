#version 450

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0, r8ui) writeonly uniform uimage2D shadingRateImage;

layout(push_constant) uniform Constants {
    vec2 mousePos; // Pixel coords.
} pcs;

uint get_rate(float dist) {
    if (dist < 50.0) return 0; // 1x1 pixels
    if (dist < 100.0) return 5; // 1x2 pixels
    return 10;                   // 2x2 pixels
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(shadingRateImage);
    
    if (texelCoord.x >= size.x || texelCoord.y >= size.y) return;

    // Transform to tile space (texel space).
    vec2 mouseTilePos = pcs.mousePos / 16.0; 
    float d = distance(vec2(texelCoord), mouseTilePos);

    uint rate = get_rate(d);
    imageStore(shadingRateImage, texelCoord, uvec4(rate, 0, 0, 0));
}