#include "theme.h"

static SDL_Color hex(Uint32 v)
{
    SDL_Color c;
    c.r = (Uint8)((v >> 16) & 0xFFu);
    c.g = (Uint8)((v >> 8) & 0xFFu);
    c.b = (Uint8)(v & 0xFFu);
    c.a = 255;
    return c;
}

Theme theme_dark(void)
{
    Theme t;
    t.background = hex(0x1E1E1Eu);
    t.display = hex(0x252525u);
    t.text = hex(0xD4D4D4u);
    t.button = hex(0x2D2D2Du);
    t.button_text = hex(0xFFFFFFu);
    t.border = hex(0x383838u);
    t.operator = hex(0x3A3A3Au);
    return t;
}

Theme theme_light(void)
{
    Theme t;
    t.background = hex(0xF2F2F2u);
    t.display = hex(0xFFFFFFu);
    t.text = hex(0x202020u);
    t.button = hex(0xE5E5E5u);
    t.button_text = hex(0x101010u);
    t.border = hex(0xD0D0D0u);
    t.operator = hex(0xD8D8D8u);
    return t;
}
