global_variable random_device gDevice;
global_variable mt19937 gRNG(gDevice());
global_variable b32 gIsDeviceSeeded;

inline r32
GetRand(r32 Low, r32 High)
{
    uniform_real_distribution<r32> Dist(Low, High);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (Dist(gRNG));
}

inline i16
GetRand(i16 Low, i16 High)
{
    uniform_int_distribution<i16> Dist(Low, High);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (Dist(gRNG));
}

inline i32
GetRand(i32 Low, i32 High)
{
    uniform_int_distribution<i32> Dist(Low, High);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (Dist(gRNG));
}

inline i64
GetRand(i64 Low, i64 High)
{
    uniform_int_distribution<i64> Dist(Low, High);
    if (gIsDeviceSeeded == false)
    {
        gIsDeviceSeeded = true;
        gRNG.seed(42);
    }
    return (Dist(gRNG));
}

internal inline v2_r32
VectorNull(void)
{
    return {0.0f, 0.0f};
}

internal inline b32
VectorIsNull(v2_r32 Vec)
{
    return (Vec.X == 0.0f && Vec.Y == 0.0f);
}

internal inline r32
Inner(v2_r32 V, v2_r32 W)
{
    return (V.X * W.X + V.Y * W.Y);
}

internal inline r32
Square(r32 X)
{
    return (X * X);
}

internal inline v2_r32
VectorRotate(v2_r32 Vec, v2_r32 Origin, r32 Rotation)
{
    // Translate it by origin
    Vec = Vec - Origin;

    // Rotate it
    r32 X = Vec.X;
    Vec.X = X * cos(Rotation) - Vec.Y * sin(Rotation);
    Vec.Y = X * sin(Rotation) + Vec.Y * cos(Rotation);

    // Translate it back by origin
    Vec = Vec + Origin;

    return (Vec);
}

internal inline r32
VectorLen(v2_r32 Vec)
{
    return (sqrt(Inner(Vec, Vec)));
}

internal inline v2_r32
VectorNormalize(v2_r32 Vec)
{
    v2_r32 Result = Vec;

    r32 Len = VectorLen(Result);
    Result.X /= Len;
    Result.Y /= Len;

    return (Result);
}

internal inline v2_r32
VectorHadamard(v2_r32 V, v2_r32 W)
{
    V.X *= W.X;
    V.Y *= W.Y;

    return (V);
}

internal inline v2_r32
Clamp(v2_r32 Vec, r32 Min, r32 Max)
{
    if (Vec.X < Min) Vec.X = Min;
    if (Vec.Y < Min) Vec.Y = Min;
    if (Vec.X > Max) Vec.X = Max;
    if (Vec.Y > Max) Vec.Y = Max;

    return (Vec);
}

internal inline v2_r32
VectorNormal(v2_r32 Vec)
{
    r32 Tmp = Vec.X;
    Vec.X = -Vec.Y;
    Vec.Y = Tmp;

    return (Vec);
}

internal inline v4_u8
Clamp(v4_u8 color, u32 Min, u32 Max)
{
    if (color.R < Min) color.R = Min;
    if (color.G < Min) color.G = Min;
    if (color.B < Min) color.B = Min;
    if (color.A < Min) color.A = Min;
    if (color.R > Max) color.R = Max;
    if (color.G > Max) color.G = Max;
    if (color.B > Max) color.B = Max;
    if (color.A > Max) color.A = Max;

    return (color);
}

internal inline r32
Clamp(r32 X, r32 Min, r32 Max)
{
    if (X < Min) X = Min;
    if (X > Max) X = Max;

    return (X);
}

internal inline b32
PointVsRec(v2_r32 P, v4_r32 Rec)
{
    return (P.X >= Rec.X && P.X <= Rec.X + Rec.W &&
            P.Y >= Rec.Y && P.Y <= Rec.Y + Rec.H);
}

internal inline b32
PointVsRec(v2_r32 Point, r32 RecX, r32 RecY, r32 RecW, r32 RecH)
{
    return (Point.X >= RecX && Point.X <= RecX + RecW && Point.Y >= RecY && Point.Y <= RecY + RecH);
}

internal inline b32
CircleVsCircle(v2_r32 P, r32 RadiusP, v2_r32 Q, r32 RadiusQ)
{
    v2_r32 PQ = {P.X - Q.X, P.Y - Q.Y};
    r32 DistanceSq = Inner(PQ, PQ);

    return (DistanceSq < Square(RadiusP + RadiusQ));
}

// vertices needs to follow clockwise ordered orientation
internal inline b32
CircleVsTriangle(v2_r32 O, r32 Radius, v2_r32 A, v2_r32 B, v2_r32 C, v2_r32 Circumcenter, r32 Rotation)
{
    A = VectorRotate(A, Circumcenter, Rotation);
    B = VectorRotate(B, Circumcenter, Rotation);
    C = VectorRotate(C, Circumcenter, Rotation);

    // Check if the vertices are within the circle
    if (Inner(O - A, O - A) <= Square(Radius))
    {
        return (true);
    }
    if (Inner(O - B, O - B) <= Square(Radius))
    {
        return (true);
    }
    if (Inner(O - C, O - C) <= Square(Radius))
    {
        return (true);
    }

    // Check if circle center is within triangle
    if ((B.Y - A.Y) * (O.X - A.X) - (B.X - A.X) * (O.Y - A.Y) >= 0 &&
        (C.Y - B.Y) * (O.X - B.X) - (C.X - B.X) * (O.Y - B.Y) >= 0 &&
        (A.Y - C.Y) * (O.X - C.X) - (A.X - C.X) * (O.Y - C.Y) >= 0)
    {
        return (true);
    }

    // Check if the circle intersects the edges
    // First edge
    r32 CircleProjectedOnSide = Inner(B - A, O - A);

    if (CircleProjectedOnSide > 0)
    {
        r32 SideLen = VectorLen(B - A);
        CircleProjectedOnSide /= SideLen;

        if (CircleProjectedOnSide < SideLen)
        {
            if (sqrt(Inner(O - A, O - A) - Square(CircleProjectedOnSide)) <= Radius)
            {
                return (true);
            }
        }
    }

    // Second edge
    CircleProjectedOnSide = Inner(C - B, O - B);

    if (CircleProjectedOnSide > 0)
    {
        r32 SideLen = VectorLen(C - B);
        CircleProjectedOnSide /= SideLen;

        if (CircleProjectedOnSide < SideLen)
        {
            if (sqrt(Inner(O - B, O - B) - Square(CircleProjectedOnSide)) <= Radius)
            {
                return (true);
            }
        }
    }

    // Third edge
    CircleProjectedOnSide = Inner(A - C, O - C);

    if (CircleProjectedOnSide > 0)
    {
        r32 SideLen = VectorLen(A - C);
        CircleProjectedOnSide /= SideLen;

        if (CircleProjectedOnSide < SideLen)
        {
            if (sqrt(Inner(O - C, O - C) - Square(CircleProjectedOnSide)) <= Radius)
            {
                return (true);
            }
        }
    }

    return (false);
}

internal b32
PolyVsPoly(mesh *MeshA, mesh *MeshB, v2_r32 *MinimumTranslationVectorA = 0)
{
    r32 MinOverlap = INFINITY;
    v2_r32 MinOverlapDirectionA = {};
    v2_r32 Normals[ArrayCount(mesh::VertexPositions) + ArrayCount(mesh::VertexPositions)];
// BEGIN_TIMED_BLOCK(Reserved5);
    Normals[MeshA->NumberOfVertices - 1] = VectorNormalize(VectorNormal(MeshA->VertexPositions[0] - MeshA->VertexPositions[MeshA->NumberOfVertices - 1]));
    Normals[MeshB->NumberOfVertices - 1 + MeshA->NumberOfVertices] = VectorNormalize(VectorNormal(MeshB->VertexPositions[0] - MeshB->VertexPositions[MeshB->NumberOfVertices - 1]));
    for (u32 VertexIndex = 0;
         VertexIndex < MeshA->NumberOfVertices - 1;
         ++VertexIndex)
    {
        Normals[VertexIndex] = VectorNormalize(VectorNormal(MeshA->VertexPositions[VertexIndex + 1] - MeshA->VertexPositions[VertexIndex]));
    }
    for (u32 VertexIndex = 0;
         VertexIndex < MeshB->NumberOfVertices - 1;
         ++VertexIndex)
    {
        Normals[VertexIndex + MeshA->NumberOfVertices] = VectorNormalize(VectorNormal(MeshB->VertexPositions[VertexIndex + 1] - MeshB->VertexPositions[VertexIndex]));
    }
// END_TIMED_BLOCK(Reserved5);
u32 NumberOfNormals = MeshA->NumberOfVertices + MeshB->NumberOfVertices;
    for (u32 NormalIndex = 0;
         NormalIndex < NumberOfNormals;
         ++NormalIndex)
    {
        v2_r32 Normal = Normals[NormalIndex];

// BEGIN_TIMED_BLOCK(Reserved6);
        // get min and max projections from A onto the normal
        r32 MinProjectionA = Inner(MeshA->VertexPositions[0], Normal);
        r32 MaxProjectionA = MinProjectionA;
        for (u32 i = 1;
             i < MeshA->NumberOfVertices;
             ++i)
        {
            r32 CurrentProj = Inner(MeshA->VertexPositions[i], Normal);
            if (CurrentProj < MinProjectionA) MinProjectionA = CurrentProj;
            if (CurrentProj > MaxProjectionA) MaxProjectionA = CurrentProj;
        }

        // get min and max projections from B onto the normal
        r32 MinProjectionB = Inner(MeshB->VertexPositions[0], Normal);
        r32 MaxProjectionB = MinProjectionB;
        for (u32 i = 1;
             i < MeshB->NumberOfVertices;
             ++i)
        {
            r32 CurrentProj = Inner(MeshB->VertexPositions[i], Normal);
            if (CurrentProj < MinProjectionB) MinProjectionB = CurrentProj;
            if (CurrentProj > MaxProjectionB) MaxProjectionB = CurrentProj;
        }
// END_TIMED_BLOCK(Reserved6);

// BEGIN_TIMED_BLOCK(Reserved7);
        b32 GapExists = !((MinProjectionA < MaxProjectionB && MinProjectionA > MinProjectionB) ||
             (MinProjectionB < MaxProjectionA && MinProjectionB > MinProjectionA));
// END_TIMED_BLOCK(Reserved7);
        if (GapExists)
        {
            return (false);
        }
// BEGIN_TIMED_BLOCK(Reserved8);
        if (MinimumTranslationVectorA)
        {
            float Overlap = min(abs(MaxProjectionA - MinProjectionB), abs(MaxProjectionB - MinProjectionA));

            // update minimum overlap for mtv
            if (Overlap < MinOverlap)
            {
                MinOverlap = Overlap;
                MinOverlapDirectionA = Normal;
            }
        }
// END_TIMED_BLOCK(Reserved8);
    }

    MinOverlapDirectionA *= MinOverlap;
    if (MinimumTranslationVectorA)
    {
        v2_r32 AB = MeshB->VertexPositions[0] - MeshA->VertexPositions[0];
        r32 OverlapDir = Inner(MinOverlapDirectionA, AB);
        if (OverlapDir > 0.0f)
        {
            MinOverlapDirectionA = -MinOverlapDirectionA;
        }
        *MinimumTranslationVectorA = MinOverlapDirectionA;
    }
    // there was an intersection in all intervals
    return (true);
}

internal inline b32
PolyVsCircle(mesh *Mesh, v2_r32 Origin, r32 Radius)
{
    for (int PointIndex = 0;
         PointIndex < Mesh->NumberOfVertices;
         ++PointIndex)
    {
        if (Inner(Mesh->VertexPositions[PointIndex] - Origin, Mesh->VertexPositions[PointIndex] - Origin) < Radius * Radius)
        {
            return (true);
        }
    }

    return (false);
}

mesh
ExtendPolygon(mesh *Original, v2_r32 Direction)
{
    mesh Result = {};

    mesh Extended = {};
    for (u32 i = 0;
         i < Original->NumberOfVertices;
         ++i)
    {
        Extended.VertexPositions[i] = Original->VertexPositions[i] + Direction;
    }

    v2_r32 Normal = VectorNormal(Direction);

    i32 MinIndex = 0;
    i32 MaxIndex = 0;
    r32 Min = Inner(Original->VertexPositions[0], Normal);
    r32 Max = Min;
    for (u32 i = 1;
         i < Original->NumberOfVertices;
         ++i)
    {
        r32 Proj = Inner(Original->VertexPositions[i], Normal);
        if (Proj < Min)
        {
            Min = Proj;
            MinIndex = i;
        }
        if (Proj > Max)
        {
            Max = Proj;
            MaxIndex = i;
        }
    }
    i32 MinIndexCpy = MinIndex;
    i32 MaxIndexCpy = MaxIndex;
    // NOTE(david): for drawing purposes this needs to be correct, for SAT purposes only checking for the normals this works either way
    i32 Add = (MinIndexCpy < MaxIndexCpy ? 1 : -1);
    while (MinIndexCpy != MaxIndexCpy)
    {
        Result.VertexPositions[Result.NumberOfVertices++] = Original->VertexPositions[MinIndexCpy];
        MinIndexCpy += Add;
        if (MinIndexCpy == Original->NumberOfVertices) MinIndexCpy = 0;
        if (MinIndexCpy < 0) MinIndexCpy = Original->NumberOfVertices - 1;
    }
    Result.VertexPositions[Result.NumberOfVertices++] = Original->VertexPositions[MinIndexCpy];
    MinIndexCpy = MinIndex;
    MaxIndexCpy = MaxIndex;
    while (MaxIndexCpy != MinIndexCpy)
    {
        Result.VertexPositions[Result.NumberOfVertices++] = Extended.VertexPositions[MaxIndexCpy];
        MaxIndexCpy += Add;
        if (MaxIndexCpy == Original->NumberOfVertices) MaxIndexCpy = 0;
        if (MaxIndexCpy < 0) MaxIndexCpy = Original->NumberOfVertices - 1;
    }
    Result.VertexPositions[Result.NumberOfVertices++] = Extended.VertexPositions[MaxIndexCpy];

    return (Result);
}

internal inline v2_r32
GetPolyOrigin(mesh *Mesh)
{
    v2_r32 Result = {};

    // NOTE(david): currently using normal average
    // normal average
    // upsides: precise, less operations
    // downsides: over/underflow possibility
    // running average
    // upsides: no over/underflow possibility
    // downsides: not precise, more operations

    for (u32 VertexIndex = 0;
         VertexIndex < Mesh->NumberOfVertices;
         ++VertexIndex)
    {
        Result += Mesh->VertexPositions[VertexIndex];
    }
    Result /= Mesh->NumberOfVertices;

    return (Result);
}

// Works for concave polygons too
internal inline b32
PolyVsPoint(mesh *Mesh, v2_r32 P)
{
    u32 Result = 0;
    
    for (u32 VertexIndexA = 0, VertexIndexB = Mesh->NumberOfVertices - 1; VertexIndexA < Mesh->NumberOfVertices; VertexIndexB = VertexIndexA++)
    {
        if ((Mesh->VertexPositions[VertexIndexA].Y > P.Y) != (Mesh->VertexPositions[VertexIndexB].Y > P.Y))
        {
            if (P.X < (Mesh->VertexPositions[VertexIndexB].X - Mesh->VertexPositions[VertexIndexA].X) * (P.Y - Mesh->VertexPositions[VertexIndexA].Y) / (Mesh->VertexPositions[VertexIndexB].Y - Mesh->VertexPositions[VertexIndexA].Y) + Mesh->VertexPositions[VertexIndexA].X)
            {
                Result = !Result;
            }
        }
    }
    return (Result);
}

// TODO(david): don't need to pass in mesh*, only vertices and their numbers
// Same goes for other functions as well
internal inline v4_r32
GetPolyAABB(mesh *Mesh)
{
    v2_r32 Min = Mesh->VertexPositions[0];
    v2_r32 Max = Mesh->VertexPositions[0];

    for (u32 VertexIndex = 1;
         VertexIndex < Mesh->NumberOfVertices;
         ++VertexIndex)
    {
        if (Mesh->VertexPositions[VertexIndex].X < Min.X) Min.X = Mesh->VertexPositions[VertexIndex].X;
        if (Mesh->VertexPositions[VertexIndex].X > Max.X) Max.X = Mesh->VertexPositions[VertexIndex].X;
        if (Mesh->VertexPositions[VertexIndex].Y < Min.Y) Min.Y = Mesh->VertexPositions[VertexIndex].Y;
        if (Mesh->VertexPositions[VertexIndex].Y > Max.Y) Max.Y = Mesh->VertexPositions[VertexIndex].Y;
    }

    return { Min.X, Min.Y, Max.X - Min.X, Max.Y - Min.Y };
}

internal inline b32
v4_r32_vs_v4_i32(v4_r32 V, v4_i32 A)
{
    return (V.X <= (r32)A.X + (r32)A.W && (r32)A.X <= V.X + V.W &&
            V.Y <= (r32)A.Y + (r32)A.H && (r32)A.Y <= V.Y + V.H);
}
