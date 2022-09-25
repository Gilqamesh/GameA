#ifndef GAME_LAYER_TYPES_HPP
# define GAME_LAYER_TYPES_HPP

struct v2_u32
{
    u32 X;
    u32 Y;
};

struct v2_r32
{
    v2_r32() = default;
    v2_r32 (r32 X, r32 Y) : X(X), Y(Y) {}
    v2_r32(Vector2 V) : X(V.x), Y(V.y) {}

    r32 X;
    r32 Y;

    operator Vector2() { return Vector2{X, Y}; }
};

struct v4_u8
{
    v4_u8() = default;
    v4_u8(u8 R, u8 G, u8 B, u8 A) : R(R), G(G), B(B), A(A) {}
    v4_u8(Color C) : R(C.r), G(C.g), B(C.b), A(C.a) {}

    u8 R;
    u8 G;
    u8 B;
    u8 A;

    operator Color() { return Color{R, G, B, A}; }
};

struct v4_r32
{
    v4_r32() = default;
    v4_r32(r32 X, r32 Y, r32 W, r32 H) : X(X), Y(Y), W(W), H(H) {}
    v4_r32(Rec R): X(R.x), Y(R.y), W(R.width), H(R.height) {}

    r32 X;
    r32 Y;
    r32 W;
    r32 H;

    operator Rec() { return Rec{X, Y, W, H}; }
};

struct v4_i32
{
    i32 X;
    i32 Y;
    i32 W;
    i32 H;

    operator Rec() { return Rec{(r32)X, (r32)Y, (r32)W, (r32)H}; }
};

struct v4_u32
{
    u32 X;
    u32 Y;
    u32 W;
    u32 H;
};

#endif
