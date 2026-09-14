#ifndef FASTCALC_THEME_H
#define FASTCALC_THEME_H

#include <SDL3/SDL.h>

/* Centralized palette. No theme engine: exactly one dark + one light set. */
typedef struct {
    SDL_Color background;
    SDL_Color display;
    SDL_Color text;
    SDL_Color button;
    SDL_Color button_text;
    SDL_Color border;
    SDL_Color operator;
} Theme;

Theme theme_dark(void);
Theme theme_light(void);

#endif
