#ifndef GAME_LAYER_HPP
# define GAME_LAYER_HPP

# include "raylib.h"
# include <random>
# include "utils.hpp"
# include "data_tags.hpp"

#if defined(G_DEBUG)
struct debug_cycle_counter
{
    u64 CycleCount;
    u32 HitCount;
};

enum
{
    DebugCycleCounter_Reserved0,
    DebugCycleCounter_Reserved1,
    DebugCycleCounter_Reserved2,
    DebugCycleCounter_Reserved3,
    DebugCycleCounter_Reserved4,
    DebugCycleCounter_Reserved5,
    DebugCycleCounter_Reserved6,
    DebugCycleCounter_Reserved7,
    DebugCycleCounter_Reserved8,
    DebugCycleCounter_Reserved9,
    DebugCycleCounter_Count
};

# define BEGIN_TIMED_BLOCK(ID) u64 StartCycleCount##ID = __rdtsc();
# define END_TIMED_BLOCK(ID) {\
    DebugGlobalInfo->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID;\
    ++DebugGlobalInfo->Counters[DebugCycleCounter_##ID].HitCount;\
}

struct debug_info
{
    u32 LastFrameCycles;
    r64 LastFrameTimeTakenInSeconds;
    u32 LastFrameTransientMemoryUsed;
    debug_cycle_counter Counters[DebugCycleCounter_Count];
};
struct debug_info *DebugGlobalInfo;

#else
# define BEGIN_TIMED_BLOCK(ID)
# define END_TIMED_BLOCK(ID)
#endif

struct memory_arena
{
    memory_index Size;
    memory_index Used;
#if defined(G_DEBUG)
    u32 UsedLastFrame;
#endif
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
internal inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    ASSERT((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
#if defined(G_DEBUG)
    Arena->UsedLastFrame += Size;
#endif

    return (Result);
}

#define PopStruct(Arena, type) {\
    ASSERT((Arena).Used >= sizeof(type));\
    (Arena).Used -= sizeof(type);\
}
#define PopArray(Arena, Count, type) {\
    ASSERT((Arena).Used >= (Count) * sizeof(type));\
    (Arena).Used -= (Count) * sizeof(type);\
}

template <typename T>
struct table
{
    T   *Data;
    u32 CurrentSize;
    u32 AllocatedSize;
};

#define PushTable(Table, ...) {\
    ASSERT((Table).CurrentSize < (Table).AllocatedSize);\
    (Table).Data[(Table).CurrentSize++] = { __VA_ARGS__ };\
}

struct tables
{
    // table<position>             Positions;
    table<velocity>             Velocities;
    table<acceleration>         Accelerations;
    table<drag>                 Drags;
    table<texture>              Textures;
    table<health>               Healths;
    table<damage>               Damages;
    table<inventory>            Inventories;
    table<item>                 Items;
    table<mesh>                 Meshes;
    table<text>                 Texts;
    table<event_add_mesh>       Events_AddMesh;
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

struct game_state
{
    memory_arena    TableArena;
    memory_arena    TransientArena;
    tables          Tables;
    // NOTE(david): could store this in tables, but did not make sense so far as part of the transform loop
    //              + haven't found an easy way yet to query for specific items yet
    fonts           Fonts;
    render_textures Canvases;
    u32             FrameCounter;
    u32             TargetFPS;
    debug_info      DebugInfo;
};

#endif
