#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(push_constant) uniform Constants {
    mat4 model;
    uint16_t handles[32];
} pushConstants;