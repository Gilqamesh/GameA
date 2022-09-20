@echo off

set CommonCompilerFlags=-Oi -O2 -FC -Zi -nologo -EHa -MD -DG_DEBUG=1
set CommonLinkerFlags=-incremental:no -opt:ref -subsystem:WINDOWS raylib.lib Kernel32.lib user32.lib opengl32.lib gdi32.lib winmm.lib Shell32.lib raylib_wrapper.lib
@REM call build.bat
pushd build
cl %CommonCompilerFlags% /Fe: GameA ../src/win32_layer.cpp /link %CommonLinkerFlags%
popd