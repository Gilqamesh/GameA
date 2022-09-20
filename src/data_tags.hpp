#ifndef DATA_TAGS_HPP
# define DATA_TAGS_HPP

# include "types.hpp"

struct position { v2_r32 p; }; // 8B

struct animation { u32 Id; u32 Frame; u32 FramesSize; u32 Reserved; }; // 16B

struct health { i32 Current; i32 Max; }; // 8B

struct mana { i32 Current; i32 Max; }; // 8B

struct energy { i32 Current; i32 Max; }; // 8B

struct behavior { u32 State; }; // 4B

struct strength { u32 Experience; }; // 4B

struct wisdom { u32 Experience; }; // 4B

struct damage { i32 Amount; }; // 4B

struct poison { i32 Amount; }; // 4B

struct woodcutting { u32 Experience; }; // 4B

struct piercing { r32 Chance; }; // 4B

struct item { char Name[16]; char Description[48]; }; // 64B

struct inventory { u32 Items[16]; }; // 64B

struct color { u8 RGBA[4]; }; // 4B

struct archery { u32 Experience; }; // 4B

struct velocity { v2_r32 d; }; // 8B

struct acceleration { v2_r32 dd; }; // 8B

struct drag { r32 Amount; }; // 4B

struct texture { Texture2D Data; v2_r32 Offset; u32 Reserved; }; // 32B

struct mesh // 64B
{
    v2_r32  VertexPositions[7];
    u32     NumberOfVertices;
    Color   color;
};

struct text
{
    u32     MeshForeignKey;
    char    Text[16];
    v2_r32  Dims;
    r32     FontSize;
    r32     Spacing;
    Color   TextColor;
    Font    font;
};

#endif
