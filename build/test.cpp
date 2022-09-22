#include "../src/platform.hpp"
#include <random>
#include "../src/raylib.h"
#include "../src/utils.hpp"
#include "../src/raylib_wrapper.cpp"
#include "../src/data_tags.hpp"
#include "../src/math.cpp"

int main()
{
    InitWindow(1280, 920, "test");

    SetTargetFPS(60);

    // v2_r32 TriangleA[3] = {
    //     { 300.0f, 300.0f },
    //     { 600.0f, 500.0f },
    //     { 450.0f, 800.0f }
    // };
    mesh MeshA = {
        {
            { 300.0f, 300.0f },
            { 600.0f, 500.0f },
            { 450.0f, 800.0f }
        },
        3
    };

    v2_r32 TriangleB[3] = {
        { 0.0f, 0.0f },
        { 200.0f, 30.0f },
        { 60.0f, 100.0f }
    };

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(WHITE);

        v2_r32 MousePos = GetMousePosition();
        mesh MeshB = {};
        MeshB.NumberOfVertices = 3;
        for (u32 i = 0; i < ArrayCount(TriangleB); ++i)
        {
            MeshB.VertexPositions[i] = TriangleB[i] + MousePos;
        }

        Color color = GREEN;

        v2_r32 MinimumTranslationVector = {};
        if (PolyVsPoly(&MeshB, &MeshA, &MinimumTranslationVector))
        {
            color = BLUE;
        }

        for (u32 i = 0; i < MeshB.NumberOfVertices; ++i)
        {
            MeshB.VertexPositions[i] += MinimumTranslationVector;
        }
        SetMousePosition(MousePos.x + MinimumTranslationVector.x, MousePos.y + MinimumTranslationVector.y);

        DrawPolygon(MeshA.VertexPositions, MeshA.NumberOfVertices, color);
        DrawPolygon(MeshB.VertexPositions, MeshB.NumberOfVertices, color);

        EndDrawing();
    }
}