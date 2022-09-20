#ifndef GAME_LAYER_HPP
# define GAME_LAYER_HPP

# include "raylib.h"
# include <random>
# include "utils.hpp"
# include "data_tags.hpp"

struct memory_arena
{
    memory_index Size;
    memory_index Used;
    u8 *Base;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count) * sizeof(type))
void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    ASSERT((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return (Result);
}

template <typename T>
struct table
{
    T   *Data;
    u32 CurrentSize;
    u32 AllocatedSize;
};

struct tables
{
    // table<position>     Positions;
    table<velocity>     Velocities;
    table<acceleration> Accelerations;
    table<drag>         Drags;
    table<texture>      Textures;
    table<health>       Healths;
    table<damage>       Damages;
    table<inventory>    Inventories;
    table<item>         Items;
    table<mesh>         Meshes;
    table<u32>          MeshClicked;
    table<u32>          MeshShown;
    table<text>         Texts;
};

struct fonts
{
    Font            Mono12;
    Font            Mono18;
    Font            Mono24;
    Font            Mono36;
    Font            Mono48;
    Font            Mono60;
    Font            Mono72;
};

struct render_textures
{
    RenderTexture2D Debug;
    RenderTexture2D Main;
};

struct debug_info
{
    u32 LastFrameCycles;
    r64 LastFrameTimeTakenInSeconds;
};

struct game_state
{
    memory_arena    TableArena;
    tables          Tables;
    // NOTE(david): could store this in tables, but did not make sense so far as part of the transform loop
    //              + haven't found an easy way yet to query for specific items yet
    fonts           Fonts;
    render_textures Canvases;
    debug_info      DebugInfo;
    u32             FrameCounter;
    u32             TargetFPS;
};

#endif
