set "SCRIPT_DIR=%~dp0"

glslc.exe "%SCRIPT_DIR%\shader.vert" -o "%SCRIPT_DIR%\vert.spv"
glslc.exe "%SCRIPT_DIR%\shader.frag" -o "%SCRIPT_DIR%\frag.spv"