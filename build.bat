@echo off

@REM -O2
set CommonCompilerFlags=-Oi -FC -Zo -Z7 -nologo -EHa -MTd -DG_DEBUG=1 /std:c++17
set CommonLinkerFlags=-incremental:no -opt:ref -subsystem:WINDOWS glfw3dll.lib raylib.lib user32.lib opengl32.lib gdi32.lib winmm.lib Shell32.lib
pushd build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ../src/game_layer.cpp -Fmgame_layer.map -LD /link -PDB:game_layer.pdb %CommonLinkerFlags% -EXPORT:UpdateAndRender -EXPORT:LoadResources -EXPORT:SetupWindows
del lock.tmp
popd
