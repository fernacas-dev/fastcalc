#include "font.h"

/* Minimal UTF-8 decoder. Returns the codepoint and advances *p past it;
 * emits U+FFFD for malformed sequences. */
static uint32_t utf8_next(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    uint32_t cp;
    if (s[0] < 0x80u) {
        cp = s[0];
        *p += 1;
    } else if ((s[0] & 0xE0u) == 0xC0u) {
        cp = ((uint32_t)(s[0] & 0x1Fu) << 6) | ((uint32_t)(s[1] & 0x3Fu));
        *p += 2;
    } else if ((s[0] & 0xF0u) == 0xE0u) {
        cp = ((uint32_t)(s[0] & 0x0Fu) << 12) |
             (((uint32_t)(s[1] & 0x3Fu)) << 6) | ((uint32_t)(s[2] & 0x3Fu));
        *p += 3;
    } else if ((s[0] & 0xF8u) == 0xF0u) {
        cp = ((uint32_t)(s[0] & 0x07u) << 18) |
             (((uint32_t)(s[1] & 0x3Fu)) << 12) |
             (((uint32_t)(s[2] & 0x3Fu)) << 6) | ((uint32_t)(s[3] & 0x3Fu));
        *p += 4;
    } else {
        cp = 0xFFFDu;
        *p += 1;
    }
    return cp;
}

bool font_load(Font *font, SDL_Renderer *renderer, const char *path, int pixel_size)
{
    font->renderer = renderer;
    font->pixel_size = pixel_size;
    font->ascent = pixel_size;
    font->loaded = false;
    glyph_cache_init(&font->cache);

    if (FT_Init_FreeType(&font->library) != 0) {
        return false;
    }
    if (FT_New_Face(font->library, path, 0, &font->face) != 0) {
        FT_Done_FreeType(font->library);
        return false;
    }
    if (!font_set_size(font, pixel_size)) {
        FT_Done_Face(font->face);
        FT_Done_FreeType(font->library);
        return false;
    }
    font->loaded = true;
    return true;
}

void font_unload(Font *font)
{
    if (!font->loaded) {
        return;
    }
    glyph_cache_clear(&font->cache, font->renderer);
    FT_Done_Face(font->face);
    FT_Done_FreeType(font->library);
    font->loaded = false;
}

bool font_set_size(Font *font, int pixel_size)
{
    if (pixel_size <= 0) {
        return false;
    }
    if (FT_Set_Pixel_Sizes(font->face, 0, (FT_UInt)pixel_size) != 0) {
        return false;
    }
    glyph_cache_clear(&font->cache, font->renderer);
    font->pixel_size = pixel_size;
    /* Baseline: ascender in pixels, fallback to ~0.8 * size. */
    FT_Face face = font->face;
    if (face->size != NULL && face->size->metrics.ascender > 0) {
        font->ascent = (int)(face->size->metrics.ascender >> 6);
    } else {
        font->ascent = (pixel_size * 4) / 5;
    }
    return true;
}

int font_text_width(Font *font, const char *text)
{
    if (text == NULL) {
        return 0;
    }
    int width = 0;
    const char *p = text;
    while (*p != '\0') {
        uint32_t cp = utf8_next(&p);
        const CachedGlyph *g =
            glyph_cache_get(&font->cache, font->renderer, font->face, cp);
        if (g == NULL) {
            continue;
        }
        width += g->advance;
    }
    return width;
}

void font_draw_text(Font *font, const char *text, float x, float y, float box_width,
                    TextAlign align, SDL_Color color)
{
    if (text == NULL || text[0] == '\0') {
        return;
    }
    int width = font_text_width(font, text);
    float pen = x;
    if (align == ALIGN_CENTER) {
        pen = x + (box_width - (float)width) * 0.5f;
    } else if (align == ALIGN_RIGHT) {
        pen = x + box_width - (float)width;
    }
    float baseline = y + (float)font->ascent;

    const char *p = text;
    while (*p != '\0') {
        uint32_t cp = utf8_next(&p);
        const CachedGlyph *g =
            glyph_cache_get(&font->cache, font->renderer, font->face, cp);
        if (g == NULL) {
            continue;
        }
        if (g->texture != NULL) {
            (void)SDL_SetTextureColorMod(g->texture, color.r, color.g, color.b);
            (void)SDL_SetTextureAlphaMod(g->texture, color.a);
            SDL_FRect dst;
            dst.x = pen + (float)g->bearing_x;
            dst.y = baseline - (float)g->bearing_y;
            dst.w = (float)g->width;
            dst.h = (float)g->height;
            (void)SDL_RenderTexture(font->renderer, g->texture, NULL, &dst);
        }
        pen += (float)g->advance;
    }
}
