@echo off

set CommonCompilerFlags=-Oi -O2 -MTd -FC -Zo -Z7 -nologo -EHa -DG_DEBUG=1
set CommonLinkerFlags=-incremental:no -opt:ref -subsystem:WINDOWS Kernel32.lib user32.lib opengl32.lib gdi32.lib winmm.lib Shell32.lib raylib_wrapper.lib
pushd build
cl %CommonCompilerFlags% /Fe: GameA ../src/win32_layer.cpp -Fmwin32_layer.map /link -PDB:win32_layer.pdb %CommonLinkerFlags%
popd