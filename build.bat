@echo off

set CommonCompilerFlags=-Oi -O2 -FC -Zo -Z7 -nologo -EHa -MD -DG_DEBUG=1 /std:c++17
set CommonLinkerFlags=-incremental:no -opt:ref -subsystem:WINDOWS raylib.lib user32.lib opengl32.lib gdi32.lib winmm.lib Shell32.lib
pushd build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ../src/game_layer.cpp -Fmgame_layer.map -LD /link %CommonLinkerFlags% -EXPORT:UpdateAndRender -EXPORT:LoadResources -EXPORT:SetupWindows
del lock.tmp
popd