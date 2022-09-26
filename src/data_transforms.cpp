#include "data_transforms.hpp"

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
    for (u32 MeshIndex = 0;
         MeshIndex < MeshesSize;
         ++MeshIndex)
    {
        mesh *Mesh = &GameState->Tables.Meshes.Data[MeshIndex];
        u32 MeshFlags = GameState->Tables.Meshes.Data[MeshIndex].Flags;
        if (MeshFlags & Mesh_Shown)
        {
            mesh *Mesh = &GameState->Tables.Meshes.Data[MeshIndex];
            RL->DrawPolygon((Vector2*)Mesh->VertexPositions, Mesh->NumberOfVertices, WHITE);

            v2_r32 MeshOrigin = GetPolyOrigin(Mesh);
            if (GameState->WorldCamera.Box.W * 0.01f < 72)
            {
                PutTextMid(GameState, RL, to_string(MeshIndex).c_str(), MeshOrigin, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            }
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
    // TODO(david): profile this

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
    for (u32 MeshIndex = 0;
         MeshIndex < GameState->Tables.Meshes.CurrentSize;
         ++MeshIndex)
    {
        GameState->Tables.Meshes.Data[MeshIndex].Flags &= ~Mesh_Clicked;
    }
}

internal inline void
Transform_DrawTexts(game_state *GameState, raylib_wrapper_code *RL)
{
    for (u32 MeshIndex = 0;
         MeshIndex < GameState->Tables.Texts.CurrentSize;
         ++MeshIndex)
    {
        text *Text = &GameState->Tables.Texts.Data[MeshIndex];
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

internal void
InsertToNodeMesh(mesh *Meshes, node_mesh *MeshNodes, u16 MeshNodesSize, i16 MeshNodeIndex, u16 *NumberOfMeshNodes, node_mesh_container *MeshContainers, u16 MeshContainersSize,
v4_i32 NodeAABB, v4_r32 MeshAABB, u16 MeshIndex, u32 *InsertRecursiveCounter, u32 *SubdivideRecursiveCounter, u16 CurDepth, u16 *MaxDepth);

internal void
SubdivideNodeMesh(mesh *Meshes, node_mesh *MeshNodes, u16 MeshNodesSize, i16 MeshNodeIndex, u16 *NumberOfMeshNodes, node_mesh_container *MeshContainers,
    u16 MeshContainersSize, v4_i32 NodeAABB, u32 *InsertRecursiveCounter, u32 *SubdivideRecursiveCounter, u16 CurDepth, u16 *MaxDepth)
{
    ++(*SubdivideRecursiveCounter);
    i32 HalfWidth  = NodeAABB.W >> 1;
    i32 HalfHeight = NodeAABB.H >> 1;
    v4_i32 ChildrenAABB[4] = {
        { NodeAABB.X, NodeAABB.Y, HalfWidth, HalfHeight },
        { NodeAABB.X + HalfWidth, NodeAABB.Y, HalfWidth, HalfHeight },
        { NodeAABB.X, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight },
        { NodeAABB.X + HalfWidth, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight }
    };

    MeshNodes[MeshNodeIndex].Index = *NumberOfMeshNodes;
    *NumberOfMeshNodes += 4;

    for (i16 Index = 0;
         Index < 4;
         ++Index)
    {
        i16 ChildNodeIndex = MeshNodes[MeshNodeIndex].Index + Index;
        MeshNodes[ChildNodeIndex].Index = ChildNodeIndex;
        MeshNodes[ChildNodeIndex].NOfMeshes = 0;
        for (i16 KeyIndex = 0;
            KeyIndex < MeshNodes[MeshNodeIndex].NOfMeshes;
            ++KeyIndex)
        {
            ASSERT(MeshNodeIndex < MeshContainersSize);
            i16 MeshIndex = MeshContainers[MeshNodeIndex].MeshForeignKey[KeyIndex];
            v4_r32 MeshAABB = GetPolyAABB(&Meshes[MeshIndex]);
            if (v4_r32_vs_v4_i32(MeshAABB, ChildrenAABB[Index]))
            {
                InsertToNodeMesh(Meshes, MeshNodes, MeshNodesSize, ChildNodeIndex, NumberOfMeshNodes, MeshContainers, MeshContainersSize,
                    ChildrenAABB[Index], MeshAABB, MeshIndex, InsertRecursiveCounter, SubdivideRecursiveCounter, CurDepth + 1, MaxDepth);
            }
        }
    }

    MeshNodes[MeshNodeIndex].NOfMeshes = -1;
}

// TODO(david): use free list for the MeshContainers instead of table
internal void
InsertToNodeMesh(mesh *Meshes, node_mesh *MeshNodes, u16 MeshNodesSize, i16 MeshNodeIndex, u16 *NumberOfMeshNodes, node_mesh_container *MeshContainers, u16 MeshContainersSize,
v4_i32 NodeAABB, v4_r32 MeshAABB, u16 MeshIndex, u32 *InsertRecursiveCounter, u32 *SubdivideRecursiveCounter, u16 CurDepth, u16 *MaxDepth)
{
    ++(*InsertRecursiveCounter);
    if (CurDepth > *MaxDepth)
    {
        *MaxDepth = CurDepth;
    }
    // TODO(david): think about what to do when the space cannot be subdivided anymore
    if (NodeAABB.W <= 1 || NodeAABB.H <= 1)
    {
        return ;
    }

    i16 NOfMeshes = MeshNodes[MeshNodeIndex].NOfMeshes;
    if (NOfMeshes >= 0) // check if current node is leaf
    {
        if (NOfMeshes < ArrayCount(node_mesh_container::MeshForeignKey))
        {
            MeshContainers[MeshNodeIndex].MeshForeignKey[NOfMeshes] = MeshIndex;
            ++MeshNodes[MeshNodeIndex].NOfMeshes;
            return ;
        }
        // run out of mesh nodes to add
        if (!(*NumberOfMeshNodes < MeshContainersSize))
        {
            return ;
        }
        SubdivideNodeMesh(Meshes, MeshNodes, MeshNodesSize, MeshNodeIndex, NumberOfMeshNodes, MeshContainers, MeshContainersSize,
            NodeAABB, InsertRecursiveCounter, SubdivideRecursiveCounter, CurDepth, MaxDepth);
    }

    i32 HalfWidth  = NodeAABB.W >> 1;
    i32 HalfHeight = NodeAABB.H >> 1;
    v4_i32 ChildrenAABB[4] = {
        { NodeAABB.X, NodeAABB.Y, HalfWidth, HalfHeight },
        { NodeAABB.X + HalfWidth, NodeAABB.Y, HalfWidth, HalfHeight },
        { NodeAABB.X, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight },
        { NodeAABB.X + HalfWidth, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight }
    };
    for (i16 Index = 0;
         Index < 4;
         ++Index)
    {
        if (v4_r32_vs_v4_i32(MeshAABB, ChildrenAABB[Index]))
        {
            i16 ChildNodeIndex = MeshNodes[MeshNodeIndex].Index + Index;
            InsertToNodeMesh(Meshes, MeshNodes, MeshNodesSize, ChildNodeIndex, NumberOfMeshNodes, MeshContainers, MeshContainersSize,
                ChildrenAABB[Index], MeshAABB, MeshIndex, InsertRecursiveCounter, SubdivideRecursiveCounter, CurDepth + 1, MaxDepth);
        }
    }
}

internal void
DebugDrawMeshNodes(game_state *GameState, node_mesh *MeshNodes, u32 MeshNodeIndex, u32 NumberOfMeshNodes, v4_i32 NodeAABB, raylib_wrapper_code *RL,
    u32 InsertRecursiveCounter, u32 SubdivideRecursiveCounter, u16 MaxDepth)
{
    r32 WidthRatio = NodeAABB.W / GameState->WorldCamera.Box.W;
    if (WidthRatio > 0.005f)
    {
        if (MeshNodeIndex == 0)
        {
            PutTextTopLeft(GameState, RL, ("Number of nodes: " + to_string(NumberOfMeshNodes)).c_str(), {}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("InsertRecursiveCounter: " + to_string(InsertRecursiveCounter)).c_str(), {0.0f, 36.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("SubdivideRecursiveCounter: " + to_string(SubdivideRecursiveCounter)).c_str(), {0.0f, 72.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("Max insert depth: " + to_string(MaxDepth)).c_str(), {0.0f, 108.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
        }
        RL->DrawRectangleLinesEx(NodeAABB, GameState->WorldCamera.Box.W * 0.001f, RED);
        if (MeshNodes[MeshNodeIndex].NOfMeshes == -1)
        {
            i16 FirstChildIndex = MeshNodes[MeshNodeIndex].Index;
            ASSERT(FirstChildIndex >= 0);
            ASSERT(FirstChildIndex < NumberOfMeshNodes);
            i32 HalfWidth  = NodeAABB.W >> 1;
            i32 HalfHeight = NodeAABB.H >> 1;
            v4_i32 ChildrenAABB[4] = {
                { NodeAABB.X, NodeAABB.Y, HalfWidth, HalfHeight },
                { NodeAABB.X + HalfWidth, NodeAABB.Y, HalfWidth, HalfHeight },
                { NodeAABB.X, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight },
                { NodeAABB.X + HalfWidth, NodeAABB.Y + HalfHeight, HalfWidth, HalfHeight }
            };
            for (i16 Index = 0;
                Index < 4;
                ++Index)
            {
                DebugDrawMeshNodes(GameState, MeshNodes, FirstChildIndex + Index, NumberOfMeshNodes, ChildrenAABB[Index], RL,
                    InsertRecursiveCounter, SubdivideRecursiveCounter, MaxDepth);
            }
        }
    }
}

internal void
DebugDrawHashTableGrids(game_state *GameState, raylib_wrapper_code *RL, mesh_grid_cell *HashTable, v2_u32 HashTableDims, v2_r32 GridDims)
{
    r32 YOffset = GameState->WorldCamera.Box.Y;
    r32 XOffset = GameState->WorldCamera.Box.X + GameState->WorldCamera.Box.W / 2.0f;
    r32 HashTableFillRate = 0.0f;
    u32 NumberOfFilledGrids = 0;
    for (u32 GridIndex = 0;
         GridIndex < HashTableDims.X * HashTableDims.Y;
         ++GridIndex)
    {
        HashTableFillRate += (r32)HashTable[GridIndex].CurrentSize;
        if (HashTable[GridIndex].CurrentSize == ArrayCount(mesh_grid_cell::MeshForeignKey))
        {
            ++NumberOfFilledGrids;
        }
    }
    HashTableFillRate /= (r32)(HashTableDims.X * HashTableDims.Y) * ArrayCount(mesh_grid_cell::MeshForeignKey);

    PutTextTopLeft(GameState, RL, ("Grid dimensions: " + to_string(GridDims.X) + " " + to_string(GridDims.Y)).c_str(),
        { XOffset, YOffset }, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
    YOffset += GameState->WorldCamera.Box.H / 60.0f;

    PutTextTopLeft(GameState, RL, ("Hash Table Dimensions: " + to_string(HashTableDims.X) + " " + to_string(HashTableDims.Y)).c_str(),
        { XOffset, YOffset }, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
    YOffset += GameState->WorldCamera.Box.H / 60.0f;

    PutTextTopLeft(GameState, RL, ("Hash Table Fill Ratio: " + to_string(HashTableFillRate)).c_str(),
        { XOffset, YOffset }, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
    YOffset += GameState->WorldCamera.Box.H / 60.0f;

    PutTextTopLeft(GameState, RL, ("Number of filled grids: " + to_string(NumberOfFilledGrids)).c_str(),
        { XOffset, YOffset }, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
    YOffset += GameState->WorldCamera.Box.H / 60.0f;

    r32 WidthRatio = GridDims.X / GameState->WorldCamera.Box.W;
    if (WidthRatio > 0.005f)
    {
        for (i32 GridX = (i32)(floor(GameState->WorldCamera.Box.X / GridDims.X) * GridDims.X);
             GridX <= GameState->WorldCamera.Box.X + GameState->WorldCamera.Box.W;
             GridX += GridDims.X)
        {
            RL->DrawLineEx({ (r32)GridX, (r32)GameState->WorldCamera.Box.Y }, { (r32)GridX, (r32)(GameState->WorldCamera.Box.Y + GameState->WorldCamera.Box.H) },
                GameState->WorldCamera.Box.W * 0.001f, RED);
        }
        for (i32 GridY = (i32)floor(GameState->WorldCamera.Box.Y / GridDims.Y) * GridDims.Y;
             GridY <= GameState->WorldCamera.Box.Y + GameState->WorldCamera.Box.H;
             GridY += GridDims.Y)
        {
            RL->DrawLineEx({ (r32)GameState->WorldCamera.Box.X, (r32)GridY }, { (r32)(GameState->WorldCamera.Box.X + GameState->WorldCamera.Box.W), (r32)GridY },
                GameState->WorldCamera.Box.W * 0.001f, RED);
        }
    }
}

internal void
Transform_MeshVsMesh(game_state *GameState, raylib_wrapper_code *RL)
{
    u32 NumberOfMeshes = GameState->Tables.Meshes.CurrentSize;
    // NOTE(david): each dimension must be power of 2, unless the bitwise and is replaced with mod check during hashing
    v2_u32 HashTableDims = { 64, 64 };
    mesh_grid_cell *HashTable = PushArray(&GameState->TransientArena, HashTableDims.X * HashTableDims.Y, mesh_grid_cell);
    ZeroArray(HashTable, HashTableDims.X * HashTableDims.Y, mesh_grid_cell);

BEGIN_TIMED_BLOCK(Reserved6);
    v2_r32 GridDims = {};
    if (NumberOfMeshes)
    {
        v4_r32 PolyAABB = GetPolyAABB(&GameState->Tables.Meshes.Data[0]);
        GridDims += { PolyAABB.W, PolyAABB.H };
    }
    for (u32 MeshIndex = 1;
         MeshIndex < NumberOfMeshes;
         ++MeshIndex)
    {
        v4_r32 PolyAABB = GetPolyAABB(&GameState->Tables.Meshes.Data[MeshIndex]);
        GridDims = (GridDims * (r32)(MeshIndex - 1) + v2_r32{ PolyAABB.W, PolyAABB.H }) / (r32)MeshIndex;
    }
END_TIMED_BLOCK(Reserved6);

BEGIN_TIMED_BLOCK(Reserved8);
    for (u32 MeshIndex = 0;
         MeshIndex < NumberOfMeshes;
         ++MeshIndex)
    {
        v4_r32 PolyAABB_v4_r32 = GetPolyAABB(&GameState->Tables.Meshes.Data[MeshIndex]);
        v4_i32 PolyAABBBounds = {
            (i32)floor(PolyAABB_v4_r32.X / GridDims.X),
            (i32)floor(PolyAABB_v4_r32.Y / GridDims.Y),
            (i32)floor((PolyAABB_v4_r32.X + PolyAABB_v4_r32.W) / GridDims.X),
            (i32)floor((PolyAABB_v4_r32.Y + PolyAABB_v4_r32.H) / GridDims.Y),
        };
        for (i32 GridX = PolyAABBBounds.X;
             GridX <= PolyAABBBounds.W;
             ++GridX)
        {
            for (i32 GridY = PolyAABBBounds.Y;
                 GridY <= PolyAABBBounds.H;
                 ++GridY)
            {
                i32 ModX = GridX & (HashTableDims.X - 1);
                i32 ModY = GridY & (HashTableDims.Y - 1);
                if (ModX < 0) ModX += HashTableDims.X;
                if (ModY < 0) ModY += HashTableDims.Y;
                u32 HashValue = ModY * HashTableDims.X + ModX;
                ASSERT(HashValue < HashTableDims.X * HashTableDims.Y);
                // TODO(david): store an extra array for overflow here, can use the 'Reserved' as an index to it
                // currently can be 256 extra mesh_grid_cells stored for overflow
                if (!(HashTable[HashValue].CurrentSize < ArrayCount(mesh_grid_cell::MeshForeignKey)))
                {
                    continue ;
                }
                ASSERT(HashTable[HashValue].CurrentSize < ArrayCount(mesh_grid_cell::MeshForeignKey));
                HashTable[HashValue].MeshForeignKey[HashTable[HashValue].CurrentSize++] = MeshIndex;
            }
        }
    }
END_TIMED_BLOCK(Reserved8);

#if defined(G_DEBUG)
    if (GameState->DebugInfo.Visible)
    {
        DebugDrawHashTableGrids(GameState, RL, HashTable, HashTableDims, GridDims);
    }
#endif

BEGIN_TIMED_BLOCK(Reserved7);
    b32 ThereWasCollision = false;
    for (u32 GridIndex = 0;
         GridIndex < HashTableDims.X * HashTableDims.Y;
         ++GridIndex)
    {
        for (u32 CenterIndex = 0;
             CenterIndex < HashTable[GridIndex].CurrentSize;
             ++CenterIndex)
        {
            u32 CenterMeshIndex = HashTable[GridIndex].MeshForeignKey[CenterIndex];
            for (u32 PushedIndex = 0;
                PushedIndex < HashTable[GridIndex].CurrentSize;
                ++PushedIndex)
            {
                u32 PushedMeshIndex = HashTable[GridIndex].MeshForeignKey[PushedIndex];
                if (PushedMeshIndex != CenterMeshIndex)
                {
                    mesh *PushedMesh = &GameState->Tables.Meshes.Data[PushedMeshIndex];
                    mesh *CenterMesh = &GameState->Tables.Meshes.Data[CenterMeshIndex];
                    v2_r32 PushedMeshMinimumTranslationVector = {};
BEGIN_TIMED_BLOCK(Reserved4);
                    b32 AreOverlapped = PolyVsPoly(PushedMesh, CenterMesh, &PushedMeshMinimumTranslationVector);
END_TIMED_BLOCK(Reserved4);
                    if (AreOverlapped)
                    {
                        PushedMeshMinimumTranslationVector *= 1.01f;
                        r32 DirectionOfTranslation = Inner(PushedMesh->FlowDirection, PushedMeshMinimumTranslationVector);
                        if (DirectionOfTranslation >= 0.0f)
                        {
                            ThereWasCollision = true;
                            for (u32 VertexIndex = 0;
                                VertexIndex < PushedMesh->NumberOfVertices;
                                ++VertexIndex)
                            {
                                PushedMesh->VertexPositions[VertexIndex] += PushedMeshMinimumTranslationVector;
                            }
                            PushedMesh->FlowDirection = PushedMeshMinimumTranslationVector;
                        }
                        else
                        {
                            ThereWasCollision = true;
                            for (u32 VertexIndex = 0;
                                VertexIndex < PushedMesh->NumberOfVertices;
                                ++VertexIndex)
                            {
                                PushedMesh->VertexPositions[VertexIndex] += -PushedMeshMinimumTranslationVector;
                            }
                            PushedMesh->FlowDirection = -PushedMeshMinimumTranslationVector;
                        }
                    }
                }
            }
        }
    }
END_TIMED_BLOCK(Reserved7);

    if (ThereWasCollision == false)
    {
        for (u32 MeshIndex = 0;
            MeshIndex < NumberOfMeshes;
            ++MeshIndex)
        {
            mesh *Mesh = &GameState->Tables.Meshes.Data[MeshIndex];
            Mesh->FlowDirection = {};
        }
    }

    PopArray(GameState->TransientArena, HashTableDims.X * HashTableDims.Y, mesh_grid_cell);
}
