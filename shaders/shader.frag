#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor1;
layout(location = 1) out vec4 outColor2;
layout(location = 2) out vec4 outColor3;
layout(location = 3) out vec4 outColor4;

void main() {
    outColor1 = texture(texSampler, fragTexCoord);
    outColor2 = vec4(outColor1.r, 0.0, 0.0, 0.0);
    outColor3 = vec4(0.0, outColor1.g, 0.0, 0.0);
    outColor4 = vec4(0.0, 0.0, outColor1.b, 0.0);
}