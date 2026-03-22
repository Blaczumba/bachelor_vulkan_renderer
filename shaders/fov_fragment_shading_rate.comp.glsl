#version 450

layout(local_size_x = 16, local_size_y = 16) in;

// r8ui is standard for shading rate attachments
layout(set = 0, binding = 0, r8ui) writeonly uniform uimage2D shadingRateImage;

layout(push_constant) uniform Constants {
    uvec2 mousePos; // Pixel coords.
} pcs;

uint getRate(float dist, bool preferVertical) {
    if (dist < 20.0) return 0; 
    if (dist < 40.0) return preferVertical ? 1 : 4;
    if (dist < 60.0) return 5;
    if (dist < 95.0) return preferVertical ? 6 : 9;
    return 10;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(shadingRateImage);
    
    if (texelCoord.x >= size.x || texelCoord.y >= size.y) return;

    // Convert mouse pixel coordinates to tile coordinates (16x16 tiles)
    vec2 mouseTilePos = vec2(pcs.mousePos) / 16.0; 
    float d = distance(vec2(texelCoord), mouseTilePos);

    vec2 diff = abs(texelCoord - mouseTilePos);
    uint rate = getRate(d, diff.x > diff.y);
    
    // Store the rate in the red channel
    imageStore(shadingRateImage, texelCoord, uvec4(rate, 0, 0, 0));
}