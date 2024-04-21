#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

glslc "$SCRIPT_DIR/shader.vert" -O -o "$SCRIPT_DIR/vert.spv"
glslc "$SCRIPT_DIR/shader.frag" -O -o "$SCRIPT_DIR/frag.spv"