#ifndef UTILS_HPP
# define UTILS_HPP

global_variable random_device gDevice;
global_variable mt19937 gRNG(gDevice());
global_variable b32 gIsDeviceSeeded;

inline r32
GetRand(r32 low, r32 high)
{
    uniform_real_distribution<r32> dist(low, high);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (dist(gRNG));
}

inline i16
GetRand(i16 low, i16 high)
{
    uniform_int_distribution<i16> dist(low, high);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (dist(gRNG));
}

inline i32
GetRand(i32 low, i32 high)
{
    uniform_int_distribution<i32> dist(low, high);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (dist(gRNG));
}

inline i64
GetRand(i64 low, i64 high)
{
    uniform_int_distribution<i64> dist(low, high);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (dist(gRNG));
}

typedef Vector2 v2_r32;
struct v2_u32
{
    u32 x;
    u32 y;
};

inline v2_r32
operator+(v2_r32 v, v2_r32 w)
{
    v.x += w.x;
    v.y += w.y;

    return (v);
}

inline v2_r32 &
operator+=(v2_r32 &v, v2_r32 w)
{
    v.x += w.x;
    v.y += w.y;

    return (v);
}

inline v2_r32
operator-(v2_r32 v, v2_r32 w)
{
    v.x -= w.x;
    v.y -= w.y;

    return (v);
}

inline v2_r32 &
operator-=(v2_r32 &v, v2_r32 w)
{
    v.x -= w.x;
    v.y -= w.y;

    return (v);
}

inline v2_r32
operator-(v2_r32 v)
{
    v.x *= -1.0f;
    v.y *= -1.0f;

    return (v);
}

inline v2_r32
operator*(r32 a, v2_r32 v)
{
    v.x *= a;
    v.y *= a;

    return (v);
}

inline v2_r32
operator*(v2_r32 v, r32 a)
{
    v.x *= a;
    v.y *= a;

    return (v);
}

inline v2_r32 &
operator*=(v2_r32 &v, r32 a)
{
    v.x *= a;
    v.y *= a;

    return (v);
}

inline v2_r32
operator/(v2_r32 v, r32 a)
{
    v.x /= a;
    v.y /= a;

    return (v);
}

inline v2_r32 &
operator/=(v2_r32 &v, r32 a)
{
    v.x /= a;
    v.y /= a;

    return (v);
}

internal inline ostream &
operator<<(ostream& os, v2_r32 v)
{
    os << v.x << " " << v.y;
    return (os);
}

internal inline ostream &
operator<<(ostream &os, Rec rec)
{
    os << rec.x << " " << rec.y << " " << rec.width << " " << rec.height;

    return (os);
}

#endif
