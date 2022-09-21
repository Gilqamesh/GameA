#include "platform.hpp"
#include "game_layer.hpp"
#include "math.cpp"
#include "ui.cpp"
#include "event.cpp"
#include "data_transforms.cpp"

#if !defined(SINGLE_FILE_BUILD)
extern "C"
{
#endif

    void SetupWindows(game_memory *GameMemory, game_window *GameWindow)
    {
        game_state *GameState = (game_state *)GameMemory->PermanentStorage;

        GameState->TargetFPS = 60;
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
            
            InitializeArena(&GameState->TransientArena, GameMemory->TransientStorageSize,
                            (u8*)GameMemory->TransientStorage);

            GameState->Tables.Meshes.AllocatedSize = 65536;
            GameState->Tables.Meshes.Data = PushArray(&GameState->TableArena, GameState->Tables.Meshes.AllocatedSize, mesh);

            GameState->Tables.Texts.AllocatedSize = 16;
            GameState->Tables.Texts.Data = PushArray(&GameState->TableArena, GameState->Tables.Texts.AllocatedSize, text);

            GameState->Tables.Events_AddMesh.AllocatedSize = 1024;
            GameState->Tables.Events_AddMesh.Data = PushArray(&GameState->TableArena, GameState->Tables.Events_AddMesh.AllocatedSize, event_add_mesh);

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

        PushTable(GameState->Tables.Meshes, { { 100.0f, 100.0f }, { 300.0f, 100.0f }, { 300.0f, 160.0f }, { 100.0f, 160.0f } }, 4, Mesh_Shown | Mesh_Event_AddMesh, BLACK);
        PushTable(GameState->Tables.Texts, 0, "Add Random Mesh",
            { GameState->Tables.Meshes.Data[0].VertexPositions[1].x - GameState->Tables.Meshes.Data[0].VertexPositions[0].x,
              GameState->Tables.Meshes.Data[0].VertexPositions[2].y - GameState->Tables.Meshes.Data->VertexPositions[1].y },
              12.0f, 1.0f, BLACK, GameState->Fonts.Mono12
        );
    }

    void UpdateAndRender(game_memory *GameMemory, game_window *GameWindow)
    {
        raylib_wrapper_code *RL = &GameMemory->RaylibWrapper;
        game_state *GameState = (game_state *)GameMemory->PermanentStorage;
#if defined(G_DEBUG)
        DebugGlobalInfo = &GameState->DebugInfo;
#endif

        BEGIN_TIMED_BLOCK(Reserved0);
        r64 FrameStartTime = RL->GetTime();

        ++GameState->FrameCounter;
        r32 dt = RL->GetFrameTime();
        v2_r32 MousePosition = RL->GetMousePosition();

        Transform_MeshEventHandler_AddMesh(GameState);
        if (RL->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Transform_ClickMesh(GameState, MousePosition);
        }
        if (RL->IsMouseButtonDown(MOUSE_BUTTON_LEFT) == false)
        {
            Transform_UnclickMesh(GameState);
        }

        RL->BeginTextureMode(GameState->Canvases.Main);
        RL->ClearBackground(GRAY);
        BEGIN_TIMED_BLOCK(Reserved1);
        Transform_SortMeshesByFlags(GameState);
        END_TIMED_BLOCK(Reserved1);
        Transform_DispatchMeshByTags(GameState, RL);
        Transform_DrawTexts(GameState, RL);
        RL->EndTextureMode();

#if defined(G_DEBUG)
        RL->BeginTextureMode(GameState->Canvases.Debug);
        if (GameState->FrameCounter % (GameState->TargetFPS / 2) == 0)
        {
            RL->ClearBackground(BLANK);
            char buffer[64];
            r32 yOffset = 0.0f;
            r32 yOffsetStride = 18.0f;
            sprintf(buffer, "%-30s %15.3fM", "Clock cycles total:", GameState->DebugInfo.LastFrameCycles / 1000000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fM", "Clock cycle budget:", 2.11f * 1000.0f / GameState->TargetFPS);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fms", "Time taken:", dt * 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3f", "Fps raw:", 1.0f / (GameState->DebugInfo.LastFrameTimeTakenInSeconds));
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3f", "Fps cooked:", 1.0f / dt);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "Permanent storage size:", GameMemory->PermanentStorageSize / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "GameState size:", sizeof(game_state) / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "Total table size:", GameState->TableArena.Used / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "Transient storage size:", GameMemory->TransientStorageSize / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "Transient storage used:", GameState->TransientArena.Used / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fkB", "Transient storage last frame:", GameState->TransientArena.UsedLastFrame / 1000.0f);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15u", "Total meshes:", GameState->Tables.Meshes.CurrentSize);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15u", "Mesh events in queue:", GameState->Tables.Events_AddMesh.CurrentSize);
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;

            sprintf(buffer, "%-30s %15.3fM", "Mesh sort procedure:", GameState->DebugInfo.Counters[DebugCycleCounter_Reserved1].CycleCount / 1000000.0f);
            GameState->DebugInfo.Counters[DebugCycleCounter_Reserved1].CycleCount = 0;
            PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
            yOffset += yOffsetStride;
        }
        RL->EndTextureMode();
#endif

        RL->BeginDrawing();
        RL->ClearBackground(MAGENTA);

        RL->DrawRenderTexture(GameState->Canvases.Main);
#if defined(G_DEBUG)
        RL->DrawRenderTexture(GameState->Canvases.Debug);
#endif

        r64 FrameCurrentTime = RL->GetTime();
        GameState->DebugInfo.LastFrameTimeTakenInSeconds = FrameCurrentTime - FrameStartTime;
        END_TIMED_BLOCK(Reserved0);
        RL->EndDrawing();

        if (RL->WindowShouldClose())
        {
            GameMemory->IsRunning = false;
        }

#if defined(G_DEBUG)
        GameState->DebugInfo.LastFrameCycles = GameState->DebugInfo.Counters[DebugCycleCounter_Reserved0].CycleCount;
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved0].CycleCount = 0;
        GameState->TableArena.UsedLastFrame = 0;
        GameState->TransientArena.UsedLastFrame = 0;
#endif
    }

#if !defined(SINGLE_FILE_BUILD)
}
#endif
