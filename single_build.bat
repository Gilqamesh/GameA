@echo off

set CommonCompilerFlags=-Oi -O2 -nologo -EHa -MD
set CommonLinkerFlags=static_raylib.lib Kernel32.lib user32.lib opengl32.lib gdi32.lib winmm.lib Shell32.lib
pushd single_build
cl %CommonCompilerFlags% /Fe: GameA -DSINGLE_FILE_BUILD ../src/win32_layer.cpp %CommonLinkerFlags%
popd