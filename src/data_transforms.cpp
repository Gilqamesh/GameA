internal void
Transform_SortMeshesByFlags(game_state *GameState)
{
    i32 NumberOfMeshes = GameState->Tables.Meshes.CurrentSize;
    u32 *OldMeshFlags = PushArray(&GameState->TransientArena, NumberOfMeshes, u32);
    u32 *NewMeshFlags = PushArray(&GameState->TransientArena, NumberOfMeshes, u32);
    u32 *NewMeshKeys = PushArray(&GameState->TransientArena, NumberOfMeshes, u32); // old->new index map

    // copy old mesh keys
    for (u32 Index = 0;
         Index < NumberOfMeshes;
         ++Index)
    {
        OldMeshFlags[Index] = GameState->Tables.Meshes.Data[Index].Flags;
    }

    // generate new mesh keys using radix sort
    u32 NumberOfDigits = 32;
    for (u32 DigitIndex = 0;
         DigitIndex < NumberOfDigits - 1;
         ++DigitIndex)
    {
        u32 DigitFlag = (1 << DigitIndex);
        u32 Keys[2] = {};
        for (u32 Index = 0; Index < NumberOfMeshes; ++Index)
        {
            u32 KeyIndex = ((OldMeshFlags[Index] & DigitFlag) == DigitFlag);
            ++Keys[KeyIndex];
        }
        Keys[1] += Keys[0];
        for (i32 Index = NumberOfMeshes - 1; Index >= 0; --Index)
        {
            u32 KeyIndex = ((OldMeshFlags[Index] & DigitFlag) == DigitFlag);
            NewMeshFlags[--Keys[KeyIndex]] = OldMeshFlags[Index];
            NewMeshKeys[Keys[KeyIndex]] = Index;
        }
        for (u32 Index = 0; Index < NumberOfMeshes; ++Index)
        {
            OldMeshFlags[Index] = NewMeshFlags[Index];
        }
    }

    // sort meshes
    for (u32 Index = 0;
         Index < NumberOfMeshes;
         ++Index)
    {
        mesh Tmp = GameState->Tables.Meshes.Data[Index];
        GameState->Tables.Meshes.Data[Index] = GameState->Tables.Meshes.Data[NewMeshKeys[Index]];
        GameState->Tables.Meshes.Data[NewMeshKeys[Index]] = Tmp;
    }

    // sort tables that include MeshForeignKey
    for (u32 Index = 0;
         Index < GameState->Tables.Events_AddMesh.CurrentSize;
         ++Index)
    {
        GameState->Tables.Events_AddMesh.Data[Index].MeshForeignKey = NewMeshKeys[GameState->Tables.Events_AddMesh.Data[Index].MeshForeignKey];
    }
    for (u32 Index = 0;
         Index < GameState->Tables.Texts.CurrentSize;
         ++Index)
    {
        GameState->Tables.Texts.Data[Index].MeshForeignKey = NewMeshKeys[GameState->Tables.Texts.Data[Index].MeshForeignKey];
    }

    PopArray(GameState->TransientArena, NumberOfMeshes, u32);
    PopArray(GameState->TransientArena, NumberOfMeshes, u32);
    PopArray(GameState->TransientArena, NumberOfMeshes, u32);
}

internal void
Transform_DispatchMeshByTags(game_state *GameState, raylib_wrapper_code *RL)
{
    b32 IsMouseDown = RL->IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    b32 IsMouseClicked = RL->IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    u32 MeshesSize = GameState->Tables.Meshes.CurrentSize;
    for (u32 Index = 0;
         Index < MeshesSize;
         ++Index)
    {
        mesh *Mesh = &GameState->Tables.Meshes.Data[Index];
        u32 MeshFlags = GameState->Tables.Meshes.Data[Index].Flags;
        if (MeshFlags & Mesh_Shown)
        {
            mesh *Mesh = &GameState->Tables.Meshes.Data[Index];
            RL->DrawPolygon(Mesh->VertexPositions, Mesh->NumberOfVertices, WHITE);
        }
        if (MeshFlags & Mesh_Clicked)
        {
            if (IsMouseClicked)
            {
                if (MeshFlags & Mesh_RemovableByClick)
                {
                    Mesh->Flags &= ~Mesh_Shown;
                    MeshFlags = MeshFlags;
                }
            }
            if (IsMouseDown)
            {
                for (u32 VertexIndex = 0;
                    VertexIndex < Mesh->NumberOfVertices;
                    ++VertexIndex)
                {
                    Mesh->VertexPositions[VertexIndex] += GameState->MouseDelta;
                }
            }
            if (IsMouseClicked && (MeshFlags & Mesh_Event_AddMesh))
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
                        Mesh_RemovableByClick | Mesh_CollidesMesh,
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
                    PushEvent(&GameState->Tables.Events_AddMesh, {
                        GameState->FrameCounter + GameState->TargetFPS,
                        GameState->Tables.Meshes.CurrentSize
                    });
                    PushTable(GameState->Tables.Meshes, GeneratedMesh);
                }
            }
        }
    }
}

internal void
Transform_ClickMesh(game_state *GameState)
{
    // TODO(david): spatial partitioning

    for (u32 Iteration = 0;
         Iteration < GameState->Tables.Meshes.CurrentSize;
         ++Iteration)
    {
        if (PolyVsPoint(&GameState->Tables.Meshes.Data[Iteration], GameState->MousePosition))
        {
            GameState->Tables.Meshes.Data[Iteration].Flags |= Mesh_Clicked;
        }
    }
}

internal inline void
Transform_UnclickMesh(game_state *GameState)
{
    for (u32 Index = 0;
         Index < GameState->Tables.Meshes.CurrentSize;
         ++Index)
    {
        GameState->Tables.Meshes.Data[Index].Flags &= ~Mesh_Clicked;
    }
}

internal inline void
Transform_DrawTexts(game_state *GameState, raylib_wrapper_code *RL)
{
    for (u32 Index = 0;
         Index < GameState->Tables.Texts.CurrentSize;
         ++Index)
    {
        text *Text = &GameState->Tables.Texts.Data[Index];
        ASSERT(Text->MeshForeignKey < GameState->Tables.Meshes.CurrentSize);
        v2_r32 Position = GameState->Tables.Meshes.Data[Text->MeshForeignKey].VertexPositions[0];

        v2_r32 TextDims = RL->MeasureTextEx(Text->font, Text->Text, Text->FontSize, Text->Spacing);
        RL->RL_DrawTextEx(Text->font, Text->Text, Position + (Text->Dims - TextDims) / 2.0f, Text->FontSize, Text->Spacing, Text->TextColor);
    }
}

internal inline void
Transform_MeshEventHandler_AddMesh(game_state *GameState)
{
    if (GameState->Tables.Events_AddMesh.CurrentSize)
    {
        ASSERT(GameState->Tables.Events_AddMesh.Data[0].Target >= GameState->FrameCounter);
    }
    while (GameState->Tables.Events_AddMesh.CurrentSize &&
           GameState->Tables.Events_AddMesh.Data[0].Target == GameState->FrameCounter)
    {
        GameState->Tables.Meshes.Data[GameState->Tables.Events_AddMesh.Data[0].MeshForeignKey].Flags |= Mesh_Shown;
        PopEvent(&GameState->Tables.Events_AddMesh);
    }
}

internal inline void
Transform_MeshVsMesh(game_state *GameState)
{
    // TODO(david): spatial partitioning

    u32 NumberOfMeshes = GameState->Tables.Meshes.CurrentSize;
    u32 *OverlapQueue = PushArray(&GameState->TransientArena, NumberOfMeshes, u32);
    for (u32 Index = 0;
         Index < NumberOfMeshes;
         ++Index)
    {
        OverlapQueue[Index] = Index;
    }
    u32 OverlapQueueSize = NumberOfMeshes;
    u32 DebugNumberOfIterations = 0;

    while (OverlapQueueSize)
    {
        u32 OuterIndex = OverlapQueue[--OverlapQueueSize];
        mesh *CurrentMesh = &GameState->Tables.Meshes.Data[OuterIndex];
        if ((CurrentMesh->Flags & (Mesh_CollidesMesh | Mesh_Shown)) == (Mesh_CollidesMesh | Mesh_Shown))
        {
            for (u32 InnerIndex = 0;
                InnerIndex < NumberOfMeshes;
                ++InnerIndex)
            {
                if (OuterIndex != InnerIndex)
                {
                    mesh *ComparedToMesh = &GameState->Tables.Meshes.Data[InnerIndex];
                    if ((ComparedToMesh->Flags & (Mesh_CollidesMesh | Mesh_Shown)) == (Mesh_CollidesMesh | Mesh_Shown))
                    {
                        v2_r32 MinimumTranslationVector = {};
                        BEGIN_TIMED_BLOCK(Reserved4);
                        b32 AreOverlapped = PolyVsPoly(ComparedToMesh, CurrentMesh, &MinimumTranslationVector);
                        END_TIMED_BLOCK(Reserved4);
                        if (AreOverlapped)
                        {
                            r32 PushOutMultiplier = 1.3f;
                            for (u32 VertexIndex = 0;
                                VertexIndex < ComparedToMesh->NumberOfVertices;
                                ++VertexIndex)
                            {
                                ComparedToMesh->VertexPositions[VertexIndex] += MinimumTranslationVector * PushOutMultiplier;
                            }
                            OverlapQueue[OverlapQueueSize++] = InnerIndex;
                            break ;
                        }
                    }
                }
            }
        }

        ASSERT(++DebugNumberOfIterations < NumberOfMeshes * 4);
    }

    PopArray(GameState->TransientArena, NumberOfMeshes, u32);
}
