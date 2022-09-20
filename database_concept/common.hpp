#ifndef COMMON_HPP
# define COMMON_HPP

// this header is used for development purposes

# include <iostream>
using namespace std;
# define LOG(x) (cout << x << endl)

# if defined(_MSC_VER)
#  include <intrin.h> // for __rdtsc() intrinsic
# endif
# include <random>
# include <vector>
# include <algorithm>
# include <thread>
# include <iomanip>
# include <cstring>

# include "types.hpp"

static random_device dev;
static mt19937 rng(dev());
b32 IsRandomDeviceInitialized;

inline r32
GetRand(r32 low, r32 high)
{
    uniform_real_distribution<r32> dist(low, high);
    if (IsRandomDeviceInitialized == false)
    {
        IsRandomDeviceInitialized = true;
        rng.seed(42);
    }
    return (dist(rng));
}

inline i16
GetRand(i16 low, i16 high)
{
    uniform_int_distribution<i16> dist(low, high);
    if (IsRandomDeviceInitialized == false)
    {
        IsRandomDeviceInitialized = true;
        rng.seed(42);
    }
    return (dist(rng));
}

inline i32
GetRand(i32 low, i32 high)
{
    uniform_int_distribution<i32> dist(low, high);
    if (IsRandomDeviceInitialized == false)
    {
        IsRandomDeviceInitialized = true;
        rng.seed(42);
    }
    return (dist(rng));
}

inline i64
GetRand(i64 low, i64 high)
{
    uniform_int_distribution<i64> dist(low, high);
    if (IsRandomDeviceInitialized == false)
    {
        IsRandomDeviceInitialized = true;
        rng.seed(42);
    }
    return (dist(rng));
}

#endif
