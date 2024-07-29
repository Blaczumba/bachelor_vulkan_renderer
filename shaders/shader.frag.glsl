#version 450

layout(binding=0) uniform CameraUniform {
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform Light {
    vec3 pos;

} light;

layout(binding = 3) uniform ObjectUniform {
    mat4 model;

} object;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;

void main() {
    vec3 color = texture(texSampler, fragTexCoord).rgb;
    bool blinn = true;

    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(light.pos - fragPosition);
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(camera.pos - fragPosition);
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


    outColor = vec4(ambient + diffuse + specular, 1.0);
    outColor1 = outColor;
}
