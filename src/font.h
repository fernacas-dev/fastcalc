#ifndef FASTCALC_FONT_H
#define FASTCALC_FONT_H

#include <stdbool.h>

#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "glyph_cache.h"

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} TextAlign;

typedef struct {
    FT_Library library;
    FT_Face face;
    SDL_Renderer *renderer;
    GlyphCache cache;
    int pixel_size;
    int ascent; /* Baseline offset from the top of the line box. */
    bool loaded;
} Font;

/* Load a TrueType face at the given pixel size. Caller keeps renderer alive. */
bool font_load(Font *font, SDL_Renderer *renderer, const char *path, int pixel_size);
void font_unload(Font *font);

/* Change pixel size; clears the glyph cache (re-rasterized lazily). */
bool font_set_size(Font *font, int pixel_size);

/* Advance width of a UTF-8 string in pixels (no rendering). */
int font_text_width(Font *font, const char *text);

/* Draw UTF-8 text with its top at y, horizontally aligned within
 * (x, x + box_width). Unknown/failed glyphs are skipped, never fatal. */
void font_draw_text(Font *font, const char *text, float x, float y, float box_width,
                    TextAlign align, SDL_Color color);

#endif
