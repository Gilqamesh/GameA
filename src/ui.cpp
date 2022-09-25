internal inline void
PutTextMid(game_state *GameState, raylib_wrapper_code *RL, const char *Text, v2_r32 Position,
    r32 FontSize, v4_u8 color, Font font, r32 Spacing = 1.0f)
{
    v2_r32 TextSize = RL->MeasureTextEx(font, Text, FontSize, Spacing);
    RL->RL_DrawTextEx(font, Text, Position - TextSize / 2.0f, FontSize, Spacing, color);
}

internal inline void
PutTextTopLeft(game_state *GameState, raylib_wrapper_code *RL, const char *Text, v2_r32 Position,
    r32 FontSize, v4_u8 color, Font font, r32 Spacing = 1.0f)
{
    RL->RL_DrawTextEx(font, Text, Position, FontSize, Spacing, color);
}
