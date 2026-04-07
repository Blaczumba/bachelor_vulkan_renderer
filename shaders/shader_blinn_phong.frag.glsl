#version 450

#include "bindless.glsl"
#include "32bit_push_constants.glsl"

RegisterUniform(Light, { \
    mat4 projView; \
    vec3 pos; \
});

#define lightBufferHandle pushConstants.handles[0]
#define diffuseMapHandle pushConstants.handles[1]

layout(set=1, binding=0) uniform CameraUniform { // Dynamic uniform buffer which depends on frame in flight
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 viewDir;

} camera;

#define shadowMapHandle pushConstants.handles[4]

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 lightFragPosition;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

const int KELNER_SIZE = 9;  // size of offsets
const ivec2 offsets[] = ivec2[](
	ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1),
	ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0),
	ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1)
);

float calculateShadow() {
    vec3 lightFrag = lightFragPosition.xyz / lightFragPosition.w;
    if(lightFrag.z >= 1.0)
        return 1.0;

    float sum = textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[0])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[1])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[2])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[3])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[4])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[5])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[6])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[7])
        + textureOffset(uGlobalTexturesShadow[nonuniformEXT(shadowMapHandle)], lightFrag.xyz, offsets[8]);

    return sum / KELNER_SIZE;
}

void main() {
    vec3 color = texture(uGlobalTextures2D[nonuniformEXT(diffuseMapHandle)], fragTexCoord).rgb;
    // vec3 color = 0.6 * vec3(1.0, 1.0, 1.0);
    const bool blinn = true;

    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(GetResource(Light, lightBufferHandle).pos - fragPosition);
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(camera.viewPos - fragPosition);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }

    vec3 specular = vec3(0.3) * spec; // assuming bright white light color

    outColor = vec4(ambient + calculateShadow()*(diffuse + 0.05 * specular), 1.0);
}
