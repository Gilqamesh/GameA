#ifndef DATA_TAGS_HPP
# define DATA_TAGS_HPP

# include "common.hpp"

struct position { v2_r32 p; };

struct mesh { u32 Id; };

struct texture { u32 Id; };

struct animation { u32 Id; u32 Frame; u32 FramesSize; };

struct health { i32 Current; i32 Max; };

struct collision_dims { r32 Width; r32 Height; }; // AABB

struct mana { i32 Current; i32 Max; };

struct energy { i32 Current; i32 Max; };

struct behavior { u32 State; };

struct strength { i32 Level; };

struct wisdom { i32 Level; };

struct damage { i32 Amount; };

struct woodcutting { u32 Level; };

struct piercing { r32 Chance; };

struct item_id { u32 Id; };

struct inventory { item_id items[16]; }; // 64B / 4B = 16, for one cache line

struct color { u8 RGBA[4]; };

struct archery { u32 Level; };

struct velocity { v2_r32 d; };

struct acceleration { v2_r32 dd; };

struct drag { r32 Amount; };

#endif
