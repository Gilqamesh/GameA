#include <random>
#include "../src/platform.hpp"
#include "../src/raylib.h"
#include "../src/game_layer_types.hpp"
#include "../src/game_layer_types.cpp"
#include "../src/raylib_wrapper.cpp"
#include "../src/data_tags.hpp"
#include "../src/math.cpp"

int main()
{
    InitWindow(1280, 920, "test");

    SetTargetFPS(60);

    mesh StartMesh = {
        { { 0.0f, 0.0f }, { 100.0f, -40.0f }, { 130.0f, 40.0f }, { 60.0f, 70.0f }, { -20.0f, 40.0f } },
        5
    };

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(WHITE);

        v2_r32 MousePos = GetMousePosition();
        mesh MouseMesh = StartMesh;
        for (u32 i = 0; i < StartMesh.NumberOfVertices; ++i)
        {
            MouseMesh.VertexPositions[i] += MousePos;
        }

        Color color = RED;

        v4_i32 AABB = { 500, 400, 300, 200 };

        v4_r32 MousePolyAABB = GetPolyAABB(&MouseMesh);
        if (v4_r32_vs_v4_i32(MousePolyAABB, AABB))
        {
            color = BLUE;
        }

        DrawPolygon((Vector2*)MouseMesh.VertexPositions, MouseMesh.NumberOfVertices, color);
        DrawRectangleRec(AABB, color);
        DrawRectangleLinesEx(MousePolyAABB, 2.0f, color);

        EndDrawing();
    }
}