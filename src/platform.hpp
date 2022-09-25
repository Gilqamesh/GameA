#ifndef PLATFORMLAYER_HPP
#define PLATFORMLAYER_HPP

/***
 * This file is what the Platform Layer provides to the Game Layer
 */

# include "types.hpp"
# include "raylib_wrapper.hpp"

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

struct game_window
{
    u32 Width;
    u32 Height;
};

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
};

#endif
