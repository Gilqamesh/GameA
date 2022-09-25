inline v2_r32
operator+(v2_r32 V, v2_r32 W)
{
    V.X += W.X;
    V.Y += W.Y;

    return (V);
}

inline v2_r32 &
operator+=(v2_r32 &V, v2_r32 W)
{
    V.X += W.X;
    V.Y += W.Y;

    return (V);
}

inline v2_r32
operator-(v2_r32 V, v2_r32 W)
{
    V.X -= W.X;
    V.Y -= W.Y;

    return (V);
}

inline v2_r32 &
operator-=(v2_r32 &V, v2_r32 W)
{
    V.X -= W.X;
    V.Y -= W.Y;

    return (V);
}

inline v2_r32
operator-(v2_r32 V)
{
    V.X *= -1.0f;
    V.Y *= -1.0f;

    return (V);
}

inline v2_r32
operator*(r32 a, v2_r32 V)
{
    V.X *= a;
    V.Y *= a;

    return (V);
}

inline v2_r32
operator*(v2_r32 V, r32 a)
{
    V.X *= a;
    V.Y *= a;

    return (V);
}

inline v2_r32 &
operator*=(v2_r32 &V, r32 a)
{
    V.X *= a;
    V.Y *= a;

    return (V);
}

inline v2_r32
operator/(v2_r32 V, r32 a)
{
    V.X /= a;
    V.Y /= a;

    return (V);
}

inline v2_r32 &
operator/=(v2_r32 &V, r32 a)
{
    V.X /= a;
    V.Y /= a;

    return (V);
}

internal inline v4_u8
operator-(v4_u8 C1, v4_u8 C2)
{
    C1.R -= C2.R;
    C1.G -= C2.G;
    C1.B -= C2.B;
    C1.A -= C2.A;

    return (C1);
}

internal inline v4_u8
operator+(v4_u8 C1, v4_u8 C2)
{
    C1.R += C2.R;
    C1.G += C2.G;
    C1.B += C2.B;
    C1.A += C2.A;

    return (C1);
}

internal inline ostream &
operator<<(ostream& Os, v2_r32 V)
{
    Os << V.X << " " << V.Y;
    return (Os);
}

internal inline ostream &
operator<<(ostream &Os, v4_r32 V)
{
    Os << V.X << " " << V.Y << " " << V.W << " " << V.H;

    return (Os);
}

internal inline ostream &
operator<<(ostream& Os, v4_u8 C)
{
    Os << C.R << " " << C.G << " " << C.B << " " << C.A;

    return (Os);
}
