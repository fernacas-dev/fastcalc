#ifndef FASTCALC_LAYOUT_H
#define FASTCALC_LAYOUT_H

#include <SDL3/SDL.h>

#include "calculator.h"

typedef enum {
    BUTTON_NUMBER,
    BUTTON_OPERATOR,
    BUTTON_EQUALS,
    BUTTON_CLEAR,
    BUTTON_DECIMAL,
    BUTTON_PERCENT,
    BUTTON_SIGN,
    BUTTON_BACKSPACE
} ButtonType;

typedef struct {
    SDL_FRect rect;   /* Computed by layout_compute(). */
    const char *label;
    ButtonType type;
    int value;        /* Digit for BUTTON_NUMBER. */
    Operator op;      /* Operator for BUTTON_OPERATOR. */
} Button;

#define BUTTON_COLS 4
#define BUTTON_ROWS 5
#define BUTTON_COUNT (BUTTON_COLS * BUTTON_ROWS)

/* Layout constants, centralized. */
#define LAYOUT_PADDING 10.0f
#define LAYOUT_GAP 6.0f

/* Fill button rects from the static grid definition and report the display
 * region. Pure function of (width, height): no hard-coded positions. */
void layout_compute(Button *buttons, int width, int height, SDL_FRect *display_rect);

/* Index of the button containing (x, y), or -1. */
int layout_hit_test(const Button *buttons, float x, float y);

#endif
