#version 450

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform Light {
    mat4 projView;

    vec3 pos;

} light;

layout(binding = 3) uniform ObjectUniform {
    mat4 model;

} object;

layout(binding = 4) uniform sampler2DShadow shadowMap;
layout(binding = 5) uniform sampler2D normalMap;

layout(location = 0) in vec3 TBNfragPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 lightFragPosition;

layout(location = 3) in vec3 TBNLightPos;
layout(location = 4) in vec3 TBNViewPos;

layout(location = 0) out vec4 outColor;

const int KELNER_SIZE = 9;  // size of offsets
const ivec2 offsets[] = ivec2[](
	ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1),
	ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0),
	ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1)
);


float calculateShadow(vec3 lightDir) {
    vec3 lightFrag = lightFragPosition.xyz / lightFragPosition.w;
    if(lightFrag.z >= 1.0)
        return 1.0;

    float sum = 0.0;
    {
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[0]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[1]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[2]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[3]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[4]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[5]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[6]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[7]);
        sum += textureOffset(shadowMap, lightFrag.xyz, offsets[8]);
    }

    return sum / KELNER_SIZE;
}


void main() {
    vec3 color = texture(texSampler, fragTexCoord).rgb;
    const bool blinn = true;

    vec3 ambient = 0.12 * color;
    // diffuse
    vec3 lightDir = normalize(TBNLightPos - TBNfragPosition);
    vec3 normal = normalize(2.0 * texture(normalMap, fragTexCoord).rgb - 1.0);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(TBNViewPos - TBNfragPosition);
    float spec;
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

    vec3 specular = vec3(0.45) * spec; // assuming bright white light color
    
    outColor = vec4(ambient + calculateShadow(lightDir)*(diffuse + specular), 1.0);
    // outColor = vec4(specular, 1.0);
}
