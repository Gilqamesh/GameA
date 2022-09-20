#include "../src/platform.hpp"
#include "../src/game_layer.hpp"
#define SINGLE_FILE_BUILD
#include "../src/raylib_wrapper.cpp"

inline b32
PolyVsPoint(mesh *Mesh, v2_r32 P)
{
    u32 Result = 0;
    
    for (u32 VertexIndexA = 0, VertexIndexB = Mesh->NumberOfVertices - 1; VertexIndexA < Mesh->NumberOfVertices; VertexIndexB = VertexIndexA++)
    {
        if ((Mesh->VertexPositions[VertexIndexA].y > P.y) != (Mesh->VertexPositions[VertexIndexB].y > P.y))
        {
            if (P.x < (Mesh->VertexPositions[VertexIndexB].x - Mesh->VertexPositions[VertexIndexA].x) * (P.y - Mesh->VertexPositions[VertexIndexA].y) / (Mesh->VertexPositions[VertexIndexB].y - Mesh->VertexPositions[VertexIndexA].y) + Mesh->VertexPositions[VertexIndexA].x)
            {
                Result = !Result;
            }
        }
    }
    return (Result);
}

int main()
{
    mesh Mesh = {
        { { 100.0f, 120.0f }, { 350.0f, 120.0f }, { 450.0f, 400.0f }, { 230.0f, 320.0f }, { 180.0f, 180.0f } },
        5
    };

    InitWindow(1280, 860, "Test");

    SetTargetFPS(60);

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(BLACK);

        Color color = RED;

        if (PolyVsPoint(&Mesh, GetMousePosition()))
        {
            color = YELLOW;
        }

        DrawText("Yof4dsa8f4sa45d4fa5sd4fafsd", 1, 1, 30, RED);

        DrawPolygon(Mesh.VertexPositions, Mesh.NumberOfVertices, color);

        EndDrawing();
    }
}
