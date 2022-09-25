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

    if (!(*NumberOfMeshNodes < MeshNodesSize))
    {
        return ;
    }
    // ASSERT(*NumberOfMeshNodes < MeshNodesSize);
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
    // ASSERT(MeshNodeIndex < MeshContainersSize);
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
        // ASSERT(NodeAABB.W > 1);
        // ASSERT(NodeAABB.H > 1);
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
DebugDrawMeshNodes(game_state *GameState, node_mesh *MeshNodes, u32 MeshNodeIndex, u32 DebugNumberOfMeshNodes, v4_i32 NodeAABB, raylib_wrapper_code *RL,
    u32 InsertRecursiveCounter, u32 SubdivideRecursiveCounter, u16 MaxDepth)
{
    // NOTE(david): Screen width's
    r32 WidthRatio = NodeAABB.W / GameState->WorldCamera.Box.W;
    if (WidthRatio > 0.005f)
    {
        if (MeshNodeIndex == 0)
        {
            PutTextTopLeft(GameState, RL, ("Number of nodes: " + to_string(DebugNumberOfMeshNodes)).c_str(), {}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("InsertRecursiveCounter: " + to_string(InsertRecursiveCounter)).c_str(), {0.0f, 36.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("SubdivideRecursiveCounter: " + to_string(SubdivideRecursiveCounter)).c_str(), {0.0f, 72.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
            PutTextTopLeft(GameState, RL, ("Max insert depth: " + to_string(MaxDepth)).c_str(), {0.0f, 108.0f}, GameState->WorldCamera.Box.W * 0.01f, BLACK, GameState->Fonts.Mono72);
        }
        RL->DrawRectangleLinesEx(NodeAABB, GameState->WorldCamera.Box.W * 0.001f, RED);
        if (MeshNodes[MeshNodeIndex].NOfMeshes == -1)
        {
            i16 FirstChildIndex = MeshNodes[MeshNodeIndex].Index;
            ASSERT(FirstChildIndex >= 0);
            ASSERT(FirstChildIndex < DebugNumberOfMeshNodes);
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
                DebugDrawMeshNodes(GameState, MeshNodes, FirstChildIndex + Index, DebugNumberOfMeshNodes, ChildrenAABB[Index], RL,
                    InsertRecursiveCounter, SubdivideRecursiveCounter, MaxDepth);
            }
        }
    }
}

internal void
Transform_MeshVsMesh(game_state *GameState, raylib_wrapper_code *RL)
{
    // NOTE(david): current limitations of this spatial partitioning data structure:
    //  1. upper bound on agent size (currently handled)
    //  2. there cant be "too many" overlapping agents at one location (currently unhandled, resulting in UB)

    u32 NumberOfMeshes = GameState->Tables.Meshes.CurrentSize;
    u32 ArbitraryNumberOfNodes = 512; // 2,048 B
    node_mesh *MeshNodes = PushArray(&GameState->TransientArena, ArbitraryNumberOfNodes, node_mesh);
    ZeroArray(MeshNodes, ArbitraryNumberOfNodes, node_mesh);
    u16 NumberOfMeshNodes = 1;
    u32 ArbitraryNumberOfMeshContainers = 512; // 32,768 B
    node_mesh_container *MeshContainers = PushArray(&GameState->TransientArena, ArbitraryNumberOfMeshContainers, node_mesh_container);
    ZeroArray(MeshContainers, ArbitraryNumberOfMeshContainers, node_mesh_container);
    // TODO(david): think about the region in which we want to check collisions in.. this could be something like an extended camera
    // or a concept like simulation region
    i32 RegionHalfW = 131072;
    v4_i32 CollisionRegion = {
        -RegionHalfW, -RegionHalfW,
         2 * RegionHalfW,  2 * RegionHalfW
    };
    // NOTE(david): for limitation 1., so that agents don't straddle over too many nodes
    v2_r32 AABBUpperLimitDims = {
        200.0f,
        200.0f
    };

    u32 InsertRecursiveCounter = 0;
    u32 SubdivideRecursiveCounter = 0;
    u16 MaxInsertionDepth = 0;
    for (u32 MeshIndex = 0;
         MeshIndex < NumberOfMeshes;
         ++MeshIndex)
    {
        mesh *Mesh = &GameState->Tables.Meshes.Data[MeshIndex];
        if (Mesh->Flags & Mesh_CollidesMesh)
        {
            v4_r32 PolyAABB = GetPolyAABB(Mesh);
            if (PolyAABB.W < AABBUpperLimitDims.X && PolyAABB.H < AABBUpperLimitDims.Y)
            {
                b32 MeshIsInCollisionRegion = v4_r32_vs_v4_i32(PolyAABB, CollisionRegion);
                if (MeshIsInCollisionRegion)
                {
BEGIN_TIMED_BLOCK(Reserved8);
                    InsertToNodeMesh(GameState->Tables.Meshes.Data, MeshNodes, ArbitraryNumberOfNodes, 0, &NumberOfMeshNodes, MeshContainers,
                        ArbitraryNumberOfMeshContainers, CollisionRegion, PolyAABB, MeshIndex, &InsertRecursiveCounter, &SubdivideRecursiveCounter, 0, &MaxInsertionDepth);
END_TIMED_BLOCK(Reserved8);
                }
            }
        }
    }

#if defined(G_DEBUG)
    if (GameState->DebugInfo.Visible)
    {
        DebugDrawMeshNodes(GameState, MeshNodes, 0, NumberOfMeshNodes, CollisionRegion, RL, InsertRecursiveCounter, SubdivideRecursiveCounter, MaxInsertionDepth);
    }
#endif

    b32 ThereWasCollision = false;
    for (u32 NodeIndex = 0;
         NodeIndex < NumberOfMeshNodes;
         ++NodeIndex)
    {
        if (MeshNodes[NodeIndex].NOfMeshes >= 0)
        {
            for (u32 CenterIndex = 0;
                 CenterIndex < MeshNodes[NodeIndex].NOfMeshes;
                 ++CenterIndex)
            {
                u32 CenterMeshIndex = MeshContainers[NodeIndex].MeshForeignKey[CenterIndex];
                for (u32 PushedIndex = 0;
                    PushedIndex < MeshNodes[NodeIndex].NOfMeshes;
                    ++PushedIndex)
                {
                    u32 PushedMeshIndex = MeshContainers[NodeIndex].MeshForeignKey[PushedIndex];
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
                            PushedMeshMinimumTranslationVector *= 1.3f;
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
    }

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

    PopArray(GameState->TransientArena, ArbitraryNumberOfNodes, node_mesh);
    PopArray(GameState->TransientArena, ArbitraryNumberOfMeshContainers, node_mesh_container);

    // u32 *OverlapStack = PushArray(&GameState->TransientArena, NumberOfMeshes, u32);
    // ZeroArray(OverlapStack, NumberOfMeshes, u32);
    // for (u32 Index = 0;
    //      Index < NumberOfMeshes;
    //      ++Index)
    // {
    //     OverlapStack[Index] = Index;
    // }
    // u32 OverlapStackSize = NumberOfMeshes;
    // u32 DebugNumberOfIterations = 0;

    // while (OverlapStackSize)
    // {
    //     u32 OuterIndex = OverlapStack[--OverlapStackSize];
    //     mesh *CurrentMesh = &GameState->Tables.Meshes.Data[OuterIndex];
    //     if ((CurrentMesh->Flags & (Mesh_CollidesMesh | Mesh_Shown)) == (Mesh_CollidesMesh | Mesh_Shown))
    //     {
    //         for (u32 InnerIndex = 0;
    //             InnerIndex < NumberOfMeshes;
    //             ++InnerIndex)
    //         {
    //             if (OuterIndex != InnerIndex)
    //             {
    //                 mesh *ComparedToMesh = &GameState->Tables.Meshes.Data[InnerIndex];
    //                 if ((ComparedToMesh->Flags & (Mesh_CollidesMesh | Mesh_Shown)) == (Mesh_CollidesMesh | Mesh_Shown))
    //                 {
    //                     v2_r32 MinimumTranslationVector = {};
    //                     BEGIN_TIMED_BLOCK(Reserved4);
    //                     b32 AreOverlapped = PolyVsPoly(ComparedToMesh, CurrentMesh, &MinimumTranslationVector);
    //                     END_TIMED_BLOCK(Reserved4);
    //                     if (AreOverlapped)
    //                     {
    //                         r32 PushOutMultiplier = 1.3f;
    //                         for (u32 VertexIndex = 0;
    //                             VertexIndex < ComparedToMesh->NumberOfVertices;
    //                             ++VertexIndex)
    //                         {
    //                             ComparedToMesh->VertexPositions[VertexIndex] += MinimumTranslationVector * PushOutMultiplier;
    //                         }
    //                         ASSERT(OverlapStackSize < NumberOfMeshes);
    //                         OverlapStack[OverlapStackSize++] = InnerIndex;
    //                         break ;
    //                     }
    //                 }
    //             }
    //         }
    //     }

    //     if (++DebugNumberOfIterations == NumberOfMeshes)
    //     {
    //         // TODO(david): this kind of works like a deferred collision resolution, but think a bit more about how this could be done instead
    //         break ;
    //     }
    // }

    // PopArray(GameState->TransientArena, NumberOfMeshes, u32);
}
