#include "platform.hpp"
#include "game_layer.hpp"
#include "math.cpp"
#include "ui.cpp"
#include "data_transforms.cpp"

#if defined(G_DEBUG)
game_memory *DebugGlobalMemory;
#endif

#if !defined(SINGLE_FILE_BUILD)
extern "C"
{
#endif

void SetupWindows(game_memory *GameMemory, game_window *GameWindow)
{
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;

    GameState->TargetFPS = 60;
    GameWindow->Shown = true;
    GameWindow->Width = 1600;
    GameWindow->Height = 900;
    GameMemory->RaylibWrapper.InitWindow(GameWindow->Width, GameWindow->Height, "GameA");
    GameMemory->RaylibWrapper.SetTargetFPS(GameState->TargetFPS);
}

void LoadResources(game_memory *GameMemory, game_window *GameWindow)
{
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;
    raylib_wrapper_code *RL = &GameMemory->RaylibWrapper;
    if (GameMemory->IsMemoryInitialized == false)
    {
        InitializeArena(&GameState->TableArena, GameMemory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)GameMemory->PermanentStorage + sizeof(game_state));

        GameState->Tables.Meshes.Data = PushArray(&GameState->TableArena, 16, mesh);
        GameState->Tables.Meshes.AllocatedSize = 16;

        GameState->Tables.MeshShown.Data = PushArray(&GameState->TableArena, 16, u32);
        GameState->Tables.MeshShown.AllocatedSize = 16;

        GameState->Tables.MeshClicked.Data = PushArray(&GameState->TableArena, 16, u32);
        GameState->Tables.MeshClicked.AllocatedSize = 16;

        GameState->Tables.Texts.Data = PushArray(&GameState->TableArena, 16, text);
        GameState->Tables.Texts.AllocatedSize = 16;

        GameMemory->IsMemoryInitialized = true;
    }

    GameState->Canvases.Debug = RL->LoadRenderTexture(GameWindow->Width, GameWindow->Height);
    GameState->Canvases.Main = RL->LoadRenderTexture(GameWindow->Width, GameWindow->Height);

    GameState->Fonts.Mono12 = RL->LoadFontEx("fonts/mono.ttf", 12, 0, 2392);
    GameState->Fonts.Mono18 = RL->LoadFontEx("fonts/mono.ttf", 18, 0, 2392);
    GameState->Fonts.Mono24 = RL->LoadFontEx("fonts/mono.ttf", 24, 0, 2392);
    GameState->Fonts.Mono36 = RL->LoadFontEx("fonts/mono.ttf", 36, 0, 2392);
    GameState->Fonts.Mono48 = RL->LoadFontEx("fonts/mono.ttf", 48, 0, 2392);
    GameState->Fonts.Mono60 = RL->LoadFontEx("fonts/mono.ttf", 60, 0, 2392);
    GameState->Fonts.Mono72 = RL->LoadFontEx("fonts/mono.ttf", 72, 0, 2392);

    GameState->Tables.Meshes.CurrentSize = 1;
        GameState->Tables.Meshes.Data[0] = {
            { { 100.0f, 100.0f }, { 200.0f, 100.0f }, { 200.0f, 160.0f }, { 100.0f, 160.0f } },
            4,
            { BLACK }
        };

        GameState->Tables.MeshShown.CurrentSize = 1;
        GameState->Tables.MeshShown.Data[0] = 0;

        GameState->Tables.Texts.CurrentSize = 1;
        GameState->Tables.Texts.Data[0] = {
            0,
            "Test Button",
            { 100.0f, 60.0f },
            12.0f,
            1.0f,
            BLACK,
            GameState->Fonts.Mono12
        };
}

void UpdateAndRender(game_memory *GameMemory, game_window *GameWindow)
{
#if defined(G_DEBUG)
    DebugGlobalMemory = GameMemory;
#endif

    BEGIN_TIMED_BLOCK(Reserved0);

    raylib_wrapper_code *RL = &GameMemory->RaylibWrapper;
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;
    ++GameState->FrameCounter;

    r64 FrameStartTime = RL->GetTime();
    r32 dt = RL->GetFrameTime();
    v2_r32 MousePosition = RL->GetMousePosition();
    v2_r32 MouseDelta = RL->GetMouseDelta();

    if (RL->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Transform_ClickMesh(GameState, MousePosition);
    }
    if (RL->IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Transform_MoveMeshOnMouseHold(GameState, MouseDelta);
    }
    else
    {
        Transform_UnclickMesh(GameState);
    }

    RL->BeginTextureMode(GameState->Canvases.Debug);
    if (GameState->FrameCounter % (GameState->TargetFPS / 2) == 0)
    {
        RL->ClearBackground(BLANK);
        char buffer[64];
        r32 yOffset = 0.0f;
        r32 yOffsetStride = 18.0f;
        sprintf(buffer, "%-20s %10.3fM", "clock cycles:", GameState->DebugInfo.LastFrameCycles / 1000000.0f);
        PutTextTopLeft(GameState, RL, buffer, { 0.0f, yOffset }, 18, BLACK, GameState->Fonts.Mono18);
        yOffset += yOffsetStride;

        sprintf(buffer, "%-20s %10.3fM", "clock cycle budget:", 2.11f * 1000.0f / GameState->TargetFPS);
        PutTextTopLeft(GameState, RL, buffer, { 0.0f, yOffset }, 18, BLACK, GameState->Fonts.Mono18);
        yOffset += yOffsetStride;

        sprintf(buffer, "%-20s %10.3fms", "time taken:", dt * 1000.0f);
        PutTextTopLeft(GameState, RL, buffer, { 0.0f, yOffset }, 18, BLACK, GameState->Fonts.Mono18);
        yOffset += yOffsetStride;

        sprintf(buffer, "%-20s %10.3f", "fps:", 1.0f / (GameState->DebugInfo.LastFrameTimeTakenInSeconds));
        PutTextTopLeft(GameState, RL, buffer, { 0.0f, yOffset }, 18, BLACK, GameState->Fonts.Mono18);
        yOffset += yOffsetStride;

        sprintf(buffer, "%-20s %10.3f", "fps cooked:", 1.0f / dt);
        PutTextTopLeft(GameState, RL, buffer, { 0.0f, yOffset }, 18, BLACK, GameState->Fonts.Mono18);
        yOffset += yOffsetStride;
    }
    RL->EndTextureMode();

    RL->BeginTextureMode(GameState->Canvases.Main);
    RL->ClearBackground(GRAY);
    Transform_DrawMeshes(GameState, RL);
    Transform_DrawTexts(GameState, RL);
    RL->EndTextureMode();

    RL->BeginDrawing();
    RL->ClearBackground(MAGENTA);

    RL->DrawRenderTexture(GameState->Canvases.Main);
    RL->DrawRenderTexture(GameState->Canvases.Debug);

    r64 FrameCurrentTime = RL->GetTime();
    GameState->DebugInfo.LastFrameTimeTakenInSeconds = FrameCurrentTime - FrameStartTime;
    END_TIMED_BLOCK(Reserved0);
    RL->EndDrawing();

    if (RL->WindowShouldClose())
    {
        GameMemory->IsRunning = false;
    }

    GameState->DebugInfo.LastFrameCycles = DebugGlobalMemory->Counters[DebugCycleCounter_Reserved0].CycleCount;
    DebugGlobalMemory->Counters[DebugCycleCounter_Reserved0].CycleCount = 0;
}

#if !defined(SINGLE_FILE_BUILD)
}
#endif
