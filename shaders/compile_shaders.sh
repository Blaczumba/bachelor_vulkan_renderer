#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

glslc -fshader-stage=vert "$SCRIPT_DIR/shader.vert.glsl" -O -o "$SCRIPT_DIR/vert.spv"
glslc -fshader-stage=frag "$SCRIPT_DIR/shader.frag.glsl" -O -o "$SCRIPT_DIR/frag.spv"