internal void
Transform_ClickMesh(game_state *GameState, v2_r32 MousePosition)
{
    // TODO(david): spatial partitioning

    for (u32 Iteration = 0;
         Iteration < GameState->Tables.Meshes.CurrentSize;
         ++Iteration)
    {
        if (PolyVsPoint(&GameState->Tables.Meshes.Data[Iteration], MousePosition))
        {
            GameState->Tables.MeshClicked.Data[GameState->Tables.MeshClicked.CurrentSize++] = Iteration;
        }
    }
}

internal void
Transform_MoveMeshOnMouseHold(game_state *GameState, v2_r32 MouseDelta)
{
    ASSERT(GameState->Tables.MeshClicked.CurrentSize <= GameState->Tables.Meshes.CurrentSize);
    for (u32 Iteration = 0;
         Iteration < GameState->Tables.MeshClicked.CurrentSize;
         ++Iteration)
    {
        ASSERT(GameState->Tables.MeshClicked.Data[Iteration] < GameState->Tables.Meshes.CurrentSize);
        for (u32 VertexIndex = 0;
             VertexIndex < GameState->Tables.Meshes.Data[GameState->Tables.MeshClicked.Data[Iteration]].NumberOfVertices;
             ++VertexIndex)
        {
            GameState->Tables.Meshes.Data[GameState->Tables.MeshClicked.Data[Iteration]].VertexPositions[VertexIndex] += MouseDelta;
        }
    }
}

internal inline void
Transform_UnclickMesh(game_state *GameState)
{
    GameState->Tables.MeshClicked.CurrentSize = 0;
}

internal inline void
Transform_DrawMeshes(game_state *GameState, raylib_wrapper_code *RL)
{
    for (u32 Iteration = 0;
         Iteration < GameState->Tables.MeshShown.CurrentSize;
         ++Iteration)
    {
        ASSERT(GameState->Tables.MeshShown.Data[Iteration] < GameState->Tables.Meshes.CurrentSize);
        mesh *Mesh = &GameState->Tables.Meshes.Data[GameState->Tables.MeshShown.Data[Iteration]];
        RL->DrawPolygon(Mesh->VertexPositions, Mesh->NumberOfVertices, WHITE);
    }
}

internal inline void
Transform_DrawTexts(game_state *GameState, raylib_wrapper_code *RL)
{
    for (u32 Iteration = 0;
         Iteration < GameState->Tables.Texts.CurrentSize;
         ++Iteration)
    {
        text *Text = &GameState->Tables.Texts.Data[Iteration];
        ASSERT(Text->MeshForeignKey < GameState->Tables.Meshes.CurrentSize);
        v2_r32 Position = GameState->Tables.Meshes.Data[Text->MeshForeignKey].VertexPositions[0];

        v2_r32 TextDims = RL->MeasureTextEx(Text->font, Text->Text, Text->FontSize, Text->Spacing);
        RL->RL_DrawTextEx(Text->font, Text->Text, Position + (Text->Dims - TextDims) / 2.0f, Text->FontSize, Text->Spacing, Text->TextColor);
    }
}
