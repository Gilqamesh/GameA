#include "platform.hpp"
#include "raylib.h"
#include "game_layer_types.hpp"
#include "game_layer_types.cpp"
#if defined(G_DEBUG)
# include <intrin.h>
#endif
#include "game_layer.hpp"
#include "math.cpp"
#include "ui.cpp"
#include "event.cpp"
#include "data_transforms.cpp"
#include "rlgl.h"

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

            GameState->Tables.Meshes.AllocatedSize = 16384;
            GameState->Tables.Meshes.Data = PushArray(&GameState->TableArena, GameState->Tables.Meshes.AllocatedSize, mesh);

            GameState->Tables.Texts.AllocatedSize = 16;
            GameState->Tables.Texts.Data = PushArray(&GameState->TableArena, GameState->Tables.Texts.AllocatedSize, text);

            GameState->Tables.Events_AddMesh.AllocatedSize = 1024;
            GameState->Tables.Events_AddMesh.Data = PushArray(&GameState->TableArena, GameState->Tables.Events_AddMesh.AllocatedSize, event_add_mesh);

            GameMemory->IsMemoryInitialized = true;
        }

        GameState->Canvases.Debug = RL->LoadRenderTexture(GameWindow->Width, GameWindow->Height);
        GameState->Canvases.Main = RL->LoadRenderTexture(GameWindow->Width, GameWindow->Height);

        GameState->Fonts.Mono12 = RL->LoadFontEx("../fonts/mono.ttf", 12, 0, 2392);
        GameState->Fonts.Mono18 = RL->LoadFontEx("../fonts/mono.ttf", 18, 0, 2392);
        GameState->Fonts.Mono24 = RL->LoadFontEx("../fonts/mono.ttf", 24, 0, 2392);
        GameState->Fonts.Mono36 = RL->LoadFontEx("../fonts/mono.ttf", 36, 0, 2392);
        GameState->Fonts.Mono48 = RL->LoadFontEx("../fonts/mono.ttf", 48, 0, 2392);
        GameState->Fonts.Mono60 = RL->LoadFontEx("../fonts/mono.ttf", 60, 0, 2392);
        GameState->Fonts.Mono72 = RL->LoadFontEx("../fonts/mono.ttf", 72, 0, 2392);

        // PushTable(GameState->Tables.Meshes, { { 100.0f, 100.0f }, { 300.0f, 100.0f }, { 300.0f, 160.0f }, { 100.0f, 160.0f } }, 4, Mesh_Shown | Mesh_Event_AddMesh, BLACK);
        // PushTable(GameState->Tables.Texts, 0, "Add N Polygon",
        //     { GameState->Tables.Meshes.Data[0].VertexPositions[1].X - GameState->Tables.Meshes.Data[0].VertexPositions[0].X,
        //       GameState->Tables.Meshes.Data[0].VertexPositions[2].Y - GameState->Tables.Meshes.Data->VertexPositions[1].Y },
        //       12.0f, 1.0f, BLACK, GameState->Fonts.Mono12
        // );

        GameState->WorldCamera.Target = { (r32)GameWindow->Width / 2.0f, (r32)GameWindow->Height / 2.0f };
        GameState->WorldCamera.Offset = { (r32)GameWindow->Width / 2.0f, (r32)GameWindow->Height / 2.0f };
        GameState->WorldCamera.Scale = 1.0f;
        GameState->ZoomTarget = 1.0f;
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

        // adjust user inputs
        GameState->MousePosition = RL->GetMousePosition();
        GameState->MouseDelta = RL->GetMouseDelta();
        if (RL->IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            GameState->MouseDeltaFalloff = GameState->MouseDelta;
        }
        else
        {
            GameState->MouseDelta = GameState->MouseDeltaFalloff;
            GameState->MouseDeltaFalloff *= 0.95f;
            if (VectorLen(GameState->MouseDeltaFalloff) < 1.0f)
            {
                GameState->MouseDeltaFalloff = {};
            }
        }

        r32 MouseWheelMul = 1.0f + GetMouseWheelMove() / 7.5f;
        GameState->ZoomTarget = Clamp(GameState->ZoomTarget * MouseWheelMul, 0.005f, 100.0f);
        r32 t = 0.125f;
        if (abs(GameState->ZoomTarget - GameState->WorldCamera.Scale) < 0.001f)
        {
            t = 1.0f;
        }
        GameState->WorldCamera.Scale += t * (GameState->ZoomTarget - GameState->WorldCamera.Scale);
        
        GameState->MouseDelta /= GameState->WorldCamera.Scale;

        if (RL->IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            GameState->WorldCamera.Rotation = GameState->WorldCamera.Rotation + GameState->MouseDelta / 100.0f;
        }
        GameState->MouseDelta = VectorRotate(GameState->MouseDelta, {}, -GameState->WorldCamera.Rotation.X);
        GameState->WorldCamera.Target -= GameState->MouseDelta;

        v2_r32 CameraTopLeft = GameState->WorldCamera.Target - GameState->WorldCamera.Offset / GameState->WorldCamera.Scale;
        v2_r32 CameraDims = 2.0f * GameState->WorldCamera.Offset / GameState->WorldCamera.Scale;
        GameState->WorldCamera.Box = {
            CameraTopLeft.X,
            CameraTopLeft.Y,
            CameraDims.X,
            CameraDims.Y
        };

        GameState->MousePosition = VectorRotate(GameState->MousePosition, GameState->WorldCamera.Offset, -GameState->WorldCamera.Rotation.X);
        GameState->MousePosition /= GameState->WorldCamera.Scale;
        GameState->MousePosition += GameState->WorldCamera.Target - GameState->WorldCamera.Offset / GameState->WorldCamera.Scale;

        if (RL->IsKeyPressed(KEY_S))
        {
            GameState->ShouldSortMeshes ^= 1;
        }

        Transform_MeshEventHandler_AddMesh(GameState);
        if (RL->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Transform_ClickMesh(GameState);
        }
        if (RL->IsMouseButtonDown(MOUSE_BUTTON_LEFT) == false)
        {
            Transform_UnclickMesh(GameState);
        }

        RL->BeginTextureMode(GameState->Canvases.Main);
        RL->ClearBackground(GRAY);

        if (RL->IsKeyPressed(KEY_A))
        {
            for (u32 Iterations = 0;
                     Iterations < GameState->Tables.Events_AddMesh.AllocatedSize / GameState->TargetFPS;
                     ++Iterations)
            {
                // NOTE(david): mutates meshes table
                u32 NumberOfVertices = GetRand(3, ArrayCount(mesh::VertexPositions));
                mesh GeneratedMesh = {
                    {},
                    NumberOfVertices,
                    Mesh_RemovableByClick | Mesh_CollidesMesh | Mesh_Shown,
                    GREEN
                };
                v2_r32 VertexPosition = { GetRand(500.0f, 1100.0f), GetRand(0.0f, 1100.0f) };
                r32 Rotation = -2.0f * PI / (r32)NumberOfVertices;
                r32 VecLength = GetRand(50.0f, 100.0f);
                v2_r32 AddedVector = VectorRotate({ VecLength, 0.0f }, {}, 0.0f);
                for (u32 VertexIndex = 0;
                    VertexIndex < GeneratedMesh.NumberOfVertices;
                    ++VertexIndex)
                {
                    GeneratedMesh.VertexPositions[VertexIndex] = VertexPosition;
                    VertexPosition += AddedVector;
                    AddedVector = VectorRotate(AddedVector, {}, Rotation);
                }
                PushTable(GameState->Tables.Meshes, GeneratedMesh);
            }
        }

rlPushMatrix();
rlTranslatef(GameState->WorldCamera.Offset.X, GameState->WorldCamera.Offset.Y, 0.0f);
rlScalef(GameState->WorldCamera.Scale, GameState->WorldCamera.Scale, 0.0f);
rlRotatef(GameState->WorldCamera.Rotation.X * 180.0f / PI, 0.0f, 0.0f, 1.0f);
// rlRotatef(GameState->WorldCamera.Rotation.Y, 1.0f, 0.0f, 0.0f);
rlTranslatef(-GameState->WorldCamera.Target.X, -GameState->WorldCamera.Target.Y, 0.0f);

        RL->DrawCircleV(GameState->MousePosition, 7.5f / GameState->WorldCamera.Scale, RED);

        BEGIN_TIMED_BLOCK(Reserved1);
        if (GameState->ShouldSortMeshes)
        {
            Transform_SortMeshesByFlags(GameState);
        }
        END_TIMED_BLOCK(Reserved1);
        BEGIN_TIMED_BLOCK(Reserved2);
        Transform_DispatchMeshByTags(GameState, RL);
        END_TIMED_BLOCK(Reserved2);
        BEGIN_TIMED_BLOCK(Reserved3);
        Transform_MeshVsMesh(GameState, RL);
        END_TIMED_BLOCK(Reserved3);
        Transform_DrawTexts(GameState, RL);

rlPopMatrix();

        RL->EndTextureMode();

#if defined(G_DEBUG)
        if (RL->IsKeyPressed(KEY_D))
        {
            GameState->DebugInfo.Visible ^= 1;
        }
        if (GameState->DebugInfo.Visible)
        {
            RL->BeginTextureMode(GameState->Canvases.Debug);
            if (GameState->FrameCounter % (GameState->TargetFPS / 2) == 0)
            {
                //
                // Make these colored based on their value
                //
                RL->ClearBackground(BLANK);
                char buffer[128];
                r32 yOffset = 0.0f;
                r32 yOffsetStride = 18.0f;
                sprintf(buffer, "%-30s %4.3fM", "Clock cycles total:", GameState->DebugInfo.LastFrameCycles / 1000000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fM", "Clock cycle budget:", 2.11f * 1000.0f / GameState->TargetFPS);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fms", "Time taken:", dt * 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3f", "Fps raw:", 1.0f / (GameState->DebugInfo.LastFrameTimeTakenInSeconds));
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3f", "Fps cooked:", 1.0f / dt);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fkB", "Permanent storage size:", GameMemory->PermanentStorageSize / 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fkB", "GameState size:", sizeof(game_state) / 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fkB", "Total table size:", GameState->TableArena.Used / 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fMB", "Transient storage size:", GameMemory->TransientStorageSize / 1000000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fkB", "Transient storage used:", GameState->TransientArena.Used / 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fkB", "Transient storage last frame:", GameState->TransientArena.UsedLastFrame / 1000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %u", "Total meshes:", GameState->Tables.Meshes.CurrentSize);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %u", "Mesh events in queue:", GameState->Tables.Events_AddMesh.CurrentSize);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fM", "Mesh sort procedure:", GameState->DebugInfo.Counters[DebugCycleCounter_Reserved1].CycleCount / 1000000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s %4.3fM", "Mesh dispatch procedure:", GameState->DebugInfo.Counters[DebugCycleCounter_Reserved2].CycleCount / 1000000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;
                
                sprintf(buffer, "%-30s %4.3fM", "Mesh vs Mesh procedure:", GameState->DebugInfo.Counters[DebugCycleCounter_Reserved3].CycleCount / 1000000.0f);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;
                
                sprintf(buffer, "%-15s %4.3fM hits: %u c/h: %llu", "Poly vs poly:",
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4].CycleCount / 1000000.0f,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4].HitCount,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4].HitCount ?
                        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4].CycleCount / GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4].HitCount : 0);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-15s %4.3fM hits: %u c/h: %llu", "Mesh click check:",
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5].CycleCount / 1000000.0f,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5].HitCount,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5].HitCount ?
                        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5].CycleCount / GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5].HitCount : 0);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-15s %4.3fM hits: %u c/h: %llu", "Reserved6:",
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6].CycleCount / 1000000.0f,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6].HitCount,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6].HitCount ?
                        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6].CycleCount / GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6].HitCount : 0);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-15s %4.3fM hits: %u c/h: %llu", "Reserved7:",
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7].CycleCount / 1000000.0f,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7].HitCount,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7].HitCount ?
                        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7].CycleCount / GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7].HitCount : 0);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                sprintf(buffer, "%-15s %4.3fM hits: %u c/h: %llu", "Reserved8:",
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8].CycleCount / 1000000.0f,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8].HitCount,
                    GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8].HitCount ?
                        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8].CycleCount / GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8].HitCount : 0);
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 18, BLACK, GameState->Fonts.Mono18);
                yOffset += yOffsetStride;

                yOffset += yOffsetStride;
                yOffset += yOffsetStride;

                sprintf(buffer, "%-30s", "Press S to toggle mesh sort");
                PutTextTopLeft(GameState, RL, buffer, {0.0f, yOffset}, 24, GREEN, GameState->Fonts.Mono24);
                yOffset += yOffsetStride;
                yOffset += yOffsetStride;
                if (GameState->ShouldSortMeshes)
                {
                    PutTextTopLeft(GameState, RL, "ON", {0.0f, yOffset}, 24, GREEN, GameState->Fonts.Mono24);
                }
                else
                {
                    PutTextTopLeft(GameState, RL, "OFF", {0.0f, yOffset}, 24, RED, GameState->Fonts.Mono24);
                }
                yOffset += yOffsetStride;
            }
            RL->EndTextureMode();
        }
#endif

        RL->BeginDrawing();
        RL->ClearBackground(MAGENTA);
        // RL->BeginMode2D(GameState->Camera);

        RL->DrawRenderTexture(GameState->Canvases.Main);
#if defined(G_DEBUG)
        if (GameState->DebugInfo.Visible)
        {
            RL->DrawRenderTexture(GameState->Canvases.Debug);
        }
#endif

        r64 FrameCurrentTime = RL->GetTime();
        GameState->DebugInfo.LastFrameTimeTakenInSeconds = FrameCurrentTime - FrameStartTime;
        END_TIMED_BLOCK(Reserved0);
        // RL->EndMode2D();
        RL->EndDrawing();

        if (RL->WindowShouldClose())
        {
            GameMemory->IsRunning = false;
        }

#if defined(G_DEBUG)
        GameState->DebugInfo.LastFrameCycles = GameState->DebugInfo.Counters[DebugCycleCounter_Reserved0].CycleCount;
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved0] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved1] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved2] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved3] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved4] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved5] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved6] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved7] = {};
        GameState->DebugInfo.Counters[DebugCycleCounter_Reserved8] = {};
        GameState->TableArena.UsedLastFrame = 0;
        GameState->TransientArena.UsedLastFrame = 0;
#endif
    }

#if !defined(SINGLE_FILE_BUILD)
}
#endif
