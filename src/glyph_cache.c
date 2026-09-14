#include "glyph_cache.h"

#include <stdlib.h>

void glyph_cache_init(GlyphCache *cache)
{
    for (size_t i = 0; i < GLYPH_CACHE_SLOTS; i++) {
        cache->slots[i].codepoint = 0;
        cache->slots[i].texture = NULL;
        cache->slots[i].width = 0;
        cache->slots[i].height = 0;
        cache->slots[i].bearing_x = 0;
        cache->slots[i].bearing_y = 0;
        cache->slots[i].advance = 0;
        cache->slots[i].occupied = false;
    }
    cache->next_evict = 0;
}

void glyph_cache_clear(GlyphCache *cache, SDL_Renderer *renderer)
{
    (void)renderer;
    for (size_t i = 0; i < GLYPH_CACHE_SLOTS; i++) {
        if (cache->slots[i].occupied && cache->slots[i].texture != NULL) {
            SDL_DestroyTexture(cache->slots[i].texture);
            cache->slots[i].texture = NULL;
        }
        cache->slots[i].occupied = false;
    }
    cache->next_evict = 0;
}

/* Rasterize one glyph into a white-with-alpha texture. Space (and other
 * empty bitmaps) yields a NULL texture with valid advance metrics. */
static bool rasterize(GlyphCache *cache, CachedGlyph *slot, SDL_Renderer *renderer,
                      FT_Face face, uint32_t codepoint)
{
    if (FT_Load_Char(face, (FT_ULong)codepoint, FT_LOAD_RENDER) != 0) {
        return false;
    }
    FT_GlyphSlot g = face->glyph;

    slot->codepoint = codepoint;
    slot->width = (int)g->bitmap.width;
    slot->height = (int)g->bitmap.rows;
    slot->bearing_x = g->bitmap_left;
    slot->bearing_y = g->bitmap_top;
    slot->advance = (int)(g->advance.x >> 6);
    slot->texture = NULL;

    if (slot->width == 0 || slot->height == 0) {
        slot->occupied = true;
        return true;
    }

    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STATIC, slot->width,
                                         slot->height);
    if (tex == NULL) {
        return false;
    }
    (void)SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    /* Expand 8-bit coverage to ARGB (white, alpha = coverage). */
    size_t px_count = (size_t)slot->width * (size_t)slot->height;
    Uint32 *pixels = (Uint32 *)SDL_malloc(px_count * sizeof(Uint32));
    if (pixels == NULL) {
        SDL_DestroyTexture(tex);
        return false;
    }
    const unsigned char *src = g->bitmap.buffer;
    int pitch = g->bitmap.pitch; /* May be negative. */
    for (int y = 0; y < slot->height; y++) {
        const unsigned char *row;
        if (pitch >= 0) {
            row = src + (size_t)y * (size_t)pitch;
        } else {
            row = src + (size_t)(slot->height - 1 - y) * (size_t)(-pitch);
        }
        for (int x = 0; x < slot->width; x++) {
            Uint32 a = (Uint32)row[x];
            pixels[(size_t)y * (size_t)slot->width + (size_t)x] =
                (a << 24) | 0x00FFFFFFu;
        }
    }
    bool ok = SDL_UpdateTexture(tex, NULL, pixels, slot->width * 4);
    SDL_free(pixels);
    if (!ok) {
        SDL_DestroyTexture(tex);
        return false;
    }

    slot->texture = tex;
    slot->occupied = true;
    (void)cache;
    return true;
}

const CachedGlyph *glyph_cache_get(GlyphCache *cache, SDL_Renderer *renderer,
                                   FT_Face face, uint32_t codepoint)
{
    for (size_t i = 0; i < GLYPH_CACHE_SLOTS; i++) {
        if (cache->slots[i].occupied && cache->slots[i].codepoint == codepoint) {
            return &cache->slots[i];
        }
    }
    /* Miss: reuse a free slot, else evict round-robin. */
    CachedGlyph *slot = NULL;
    for (size_t i = 0; i < GLYPH_CACHE_SLOTS; i++) {
        if (!cache->slots[i].occupied) {
            slot = &cache->slots[i];
            break;
        }
    }
    if (slot == NULL) {
        slot = &cache->slots[cache->next_evict];
        cache->next_evict = (cache->next_evict + 1) % GLYPH_CACHE_SLOTS;
        if (slot->texture != NULL) {
            SDL_DestroyTexture(slot->texture);
            slot->texture = NULL;
        }
        slot->occupied = false;
    }
    if (!rasterize(cache, slot, renderer, face, codepoint)) {
        slot->occupied = false;
        slot->texture = NULL;
        return NULL;
    }
    return slot;
}
