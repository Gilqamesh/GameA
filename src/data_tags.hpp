#ifndef DATA_TAGS_HPP
# define DATA_TAGS_HPP

# include "types.hpp"

struct position { v2_r32 p; }; // 8B
static_assert(sizeof(position) % 8 == 0);

struct animation { u32 Id; u32 Frame; u32 FramesSize; u32 Reserved; }; // 16B
static_assert(sizeof(animation) % 16 == 0);

struct health { i32 Current; i32 Max; }; // 8B
static_assert(sizeof(health) % 8 == 0);

struct mana { i32 Current; i32 Max; }; // 8B
static_assert(sizeof(mana) % 8 == 0);

struct energy { i32 Current; i32 Max; }; // 8B
static_assert(sizeof(energy) % 8 == 0);

struct behavior { u32 State; }; // 4B
static_assert(sizeof(behavior) % 4 == 0);

struct strength { u32 Experience; }; // 4B
static_assert(sizeof(strength) % 4 == 0);

struct wisdom { u32 Experience; }; // 4B
static_assert(sizeof(wisdom) % 4 == 0);

struct damage { i32 Amount; }; // 4B
static_assert(sizeof(damage) % 4 == 0);

struct poison { i32 Amount; }; // 4B
static_assert(sizeof(poison) % 4 == 0);

struct woodcutting { u32 Experience; }; // 4B
static_assert(sizeof(woodcutting) % 4 == 0);

struct piercing { r32 Chance; }; // 4B
static_assert(sizeof(piercing) % 4 == 0);

struct item { char Name[16]; char Description[48]; }; // 64B
static_assert(sizeof(item) % 64 == 0);

struct inventory { u32 Items[16]; }; // 64B
static_assert(sizeof(inventory) % 64 == 0);

struct color { u8 RGBA[4]; }; // 4B
static_assert(sizeof(color) % 4 == 0);

struct archery { u32 Experience; }; // 4B
static_assert(sizeof(archery) % 4 == 0);

struct velocity { v2_r32 d; }; // 8B
static_assert(sizeof(velocity) % 8 == 0);

struct acceleration { v2_r32 dd; }; // 8B
static_assert(sizeof(acceleration) % 8 == 0);

struct drag { r32 Amount; }; // 4B
static_assert(sizeof(drag) % 4 == 0);

struct texture { Texture2D Data; v2_r32 Offset; u32 Reserved; }; // 32B
static_assert(sizeof(texture) % 32 == 0);

enum mesh_flags
{
    Mesh_Clicked            = (1 << 0),
    Mesh_Event_AddMesh      = (1 << 1),
    Mesh_Shown              = (1 << 2),
    Mesh_RemovableByClick   = (1 << 3),
    Mesh_CollidesMesh       = (1 << 4),
    Mesh_Reserved3          = (1 << 5),
    Mesh_Reserved4          = (1 << 6),
    Mesh_Reserved5          = (1 << 7),
    Mesh_Reserved6          = (1 << 8),
    Mesh_Reserved7          = (1 << 9),
    Mesh_Reserved8          = (1 << 10),
    Mesh_Reserved9          = (1 << 11),
};

struct mesh // 64B
{
    v2_r32  VertexPositions[5];
    u32     NumberOfVertices;
    // TODO(david): problem with storing flags here is that now we cant
    // sort meshes based on their flags to optimize for the instruction prefetcher
    // as the key of a mesh is the key in the table
    // but otherwise if the flags would be stored elsewhere then the data prefetcher
    // would suffer during dispatching the meshes based on their flags
    // as a result, I think best is to actually sort the mesh data
    // and sort all other tables which include a MeshForeignKey as well
    u32     Flags;
    Color   color;
    // NOTE(david): by storing the direction of flow, it makes for a consistent chain collision resolution
    v2_r32  FlowDirection;
    u32     Reserved;
};
static_assert(sizeof(mesh) % 64 == 0);

struct text // 128B
{
    u32     MeshForeignKey; // 4B
    char    Text[54];       // 56B
    v2_r32  Dims;           // 8B
    r32     FontSize;       // 4B
    r32     Spacing;        // 4B
    Color   TextColor;      // 4B
    Font    font;           // 48B
};
static_assert(sizeof(text) % 128 == 0);

// Events, these are managed by a heap and sorted by their 'u32 Target' member
struct event_add_mesh // 8B
{
    u32 Target;
    u32 MeshForeignKey;
};
static_assert(sizeof(event_add_mesh) % 8 == 0);

#endif
