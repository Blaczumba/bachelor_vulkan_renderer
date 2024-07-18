set "SCRIPT_DIR=%~dp0"

glslc.exe -fshader-stage=vert "%SCRIPT_DIR%\shader.vert.glsl" -O -o "%SCRIPT_DIR%\vert.spv"
glslc.exe -fshader-stage=frag "%SCRIPT_DIR%\shader.frag.glsl" -O -o "%SCRIPT_DIR%\frag.spv"

glslc.exe -fshader-stage=vert "%SCRIPT_DIR%\offscreen.vert.glsl" -O -o "%SCRIPT_DIR%\off.vert.spv"
glslc.exe -fshader-stage=frag "%SCRIPT_DIR%\offscreen.frag.glsl" -O -o "%SCRIPT_DIR%\off.frag.spv"

glslc.exe -fshader-stage=vert "%SCRIPT_DIR%\skybox.vert.glsl" -O -o "%SCRIPT_DIR%\skybox.vert.spv"
glslc.exe -fshader-stage=frag "%SCRIPT_DIR%\skybox.frag.glsl" -O -o "%SCRIPT_DIR%\skybox.frag.spv"

glslc.exe -fshader-stage=vert "%SCRIPT_DIR%\skybox_offscreen.vert.glsl" -O -o "%SCRIPT_DIR%\skybox_offscreen.vert.spv"
glslc.exe -fshader-stage=frag "%SCRIPT_DIR%\skybox_offscreen.frag.glsl" -O -o "%SCRIPT_DIR%\skybox_offscreen.frag.spv"