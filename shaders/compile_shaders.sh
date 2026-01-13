#!/bin/bash

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Compile shaders with glslc
glslc -fshader-stage=vertex "$SCRIPT_DIR/shader_blinn_phong.vert.glsl" -O -o "$SCRIPT_DIR/shader_blinn_phong.vert.spv"
glslc -fshader-stage=tesscontrol "$SCRIPT_DIR/shader_blinn_phong.tsc.glsl" -O -o "$SCRIPT_DIR/shader_blinn_phong.tsc.spv"
glslc -fshader-stage=tesseval "$SCRIPT_DIR/shader_blinn_phong.tse.glsl" -O -o "$SCRIPT_DIR/shader_blinn_phong.tse.spv"
glslc -fshader-stage=fragment "$SCRIPT_DIR/shader_blinn_phong.frag.glsl" -O -o "$SCRIPT_DIR/shader_blinn_phong.frag.spv"

glslc -fshader-stage=vertex "$SCRIPT_DIR/skybox.vert.glsl" -O -o "$SCRIPT_DIR/skybox.vert.spv"
glslc -I "$SCRIPT_DIR/bindless.glsl" -fshader-stage=fragment "$SCRIPT_DIR/skybox.frag.glsl" -O -o "$SCRIPT_DIR/skybox.frag.spv"

glslc -fshader-stage=vertex "$SCRIPT_DIR/shadow.vert.glsl" -O -o "$SCRIPT_DIR/shadow.vert.spv"
glslc -fshader-stage=fragment "$SCRIPT_DIR/shadow.frag.glsl" -O -o "$SCRIPT_DIR/shadow.frag.spv"

glslc -I "$SCRIPT_DIR/bindless.glsl" -I "%SCRIPT_DIR%\32bit_push_constants.glsl" -fshader-stage=vertex "$SCRIPT_DIR/pbr_env_mapping.vert.glsl" -O -o "$SCRIPT_DIR/pbr_env_mapping.vert.spv"
glslc -I "$SCRIPT_DIR/bindless.glsl" -I "%SCRIPT_DIR%\32bit_push_constants.glsl" -fshader-stage=vertex "$SCRIPT_DIR/shader_pbr.vert.glsl" -O -o "$SCRIPT_DIR/shader_pbr.vert.spv"

glslc -fshader-stage=vertex "$SCRIPT_DIR/shader_pbr_tesselation.vert.glsl" -O -o "$SCRIPT_DIR/shader_pbr_tesselation.vert.spv"
glslc -fshader-stage=tesscontrol "$SCRIPT_DIR/shader_pbr_tesselation.tsc.glsl" -O -o "$SCRIPT_DIR/shader_pbr_tesselation.tsc.spv"
glslc -fshader-stage=tesseval "$SCRIPT_DIR/shader_pbr_tesselation.tse.glsl" -O -o "$SCRIPT_DIR/shader_pbr_tesselation.tse.spv"
glslc -fshader-stage=fragment "$SCRIPT_DIR/shader_pbr_tesselation.frag.glsl" -O -o "$SCRIPT_DIR/shader_pbr_tesselation.frag.spv"

glslc -I "$SCRIPT_DIR/bindless.glsl" -I "%SCRIPT_DIR%\32bit_push_constants.glsl" -fshader-stage=fragment "$SCRIPT_DIR/shader_pbr.frag.glsl" -O -o "$SCRIPT_DIR/shader_pbr.frag.spv"

