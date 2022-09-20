#ifndef PLATFORMLAYER_HPP
#define PLATFORMLAYER_HPP

/***
 * This file is what the Platform Layer provides to the Game Layer
 */

# include "types.hpp"
# include <string>

# if defined(G_DEBUG)
#  include <intrin.h>
# endif

# define internal static
# define local_persist static
# define global_variable static

# include <cassert>

# if defined(G_DEBUG)
#  define ASSERT(Expression) if(!(Expression)) { LOG(#Expression); assert(Expression); }
# else
#  define ASSERT(Expression)
# endif

# define Kilobytes(Value) ((Value)*1024LL)
# define Megabytes(Value) (Kilobytes(Value)*1024LL)
# define Gigabytes(Value) (Megabytes(Value)*1024LL)
# define Terabytes(Value) (Gigabytes(Value)*1024LL)

# define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

# define LOG(x) (cout << x << endl)
# define LINE() (LOG(__FILE__ << " " << __LINE__))

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
    DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID;\
    ++DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount;\
}
extern struct game_memory *DebugGlobalMemory;
#else
# define BEGIN_TIMED_BLOCK(ID)
# define END_TIMED_BLOCK(ID)
#endif

struct game_window
{
    u16 Width;
    u16 Height;
    b32 Shown;
};

# include "raylib_wrapper.hpp"

typedef void (*platform_sleep)(u32);

struct game_memory
{
    b32 IsMemoryInitialized;
    b32 IsRunning;

    raylib_wrapper_code RaylibWrapper;

    void *PermanentStorage;
    u64 PermanentStorageSize;
    void *TransientStorage;
    u64 TransientStorageSize;

#if defined(G_DEBUG)
    debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif
};

#endif
