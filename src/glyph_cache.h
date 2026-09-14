#ifndef FASTCALC_GLYPH_CACHE_H
#define FASTCALC_GLYPH_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

/* Fixed-size MRU texture cache: each distinct codepoint is rasterized once
 * per font size. Small linear search is plenty (calculator alphabet is tiny)
 * and avoids any hashing allocator churn in the render path. */
#define GLYPH_CACHE_SLOTS 256

typedef struct {
    uint32_t codepoint;
    SDL_Texture *texture; /* White glyph with alpha; tinted via color mod. */
    int width;
    int height;
    int bearing_x;
    int bearing_y;
    int advance; /* Horizontal advance in pixels. */
    bool occupied;
} CachedGlyph;

typedef struct {
    CachedGlyph slots[GLYPH_CACHE_SLOTS];
    unsigned next_evict;
} GlyphCache;

void glyph_cache_init(GlyphCache *cache);
/* Destroy all textures; keeps slots reusable. */
void glyph_cache_clear(GlyphCache *cache, SDL_Renderer *renderer);

/* Look up (and rasterize on miss) a codepoint. Returns NULL on failure;
 * the pointer is owned by the cache and stays valid until clear(). */
const CachedGlyph *glyph_cache_get(GlyphCache *cache, SDL_Renderer *renderer,
                                   FT_Face face, uint32_t codepoint);

#endif
