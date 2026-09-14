#ifndef FASTCALC_APP_H
#define FASTCALC_APP_H

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "calculator.h"
#include "font.h"
#include "layout.h"
#include "theme.h"

#define APP_INIT_WIDTH 420
#define APP_INIT_HEIGHT 600
#define APP_MIN_WIDTH 320
#define APP_MIN_HEIGHT 480

#define APP_DISPLAY_FONT_PX 42
#define APP_BUTTON_FONT_PX 24
#define APP_FONT_MIN_PX 16
#define APP_FONT_MAX_PX 72

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;

    CalculatorState calc;

    Theme theme;
    bool dark_mode;

    Font display_font;
    Font button_font;

    Button buttons[BUTTON_COUNT];
    SDL_FRect display_rect;

    bool running;
    bool needs_redraw;

    int window_width;
    int window_height;

    int hover_button;   /* -1 when the pointer is not over a button. */
    int pressed_button; /* -1 when no button is held down. */

    int font_steps; /* Font-size adjustment steps around the default. */

    bool text_input_active; /* Printable keys handled via TEXT_INPUT events. */

    char display_buffer[128];
    char pending_buffer[96];
};
typedef struct App App;

/* Full lifecycle: SDL -> window -> renderer -> fonts -> state. */
bool app_init(App *app);
void app_run(App *app);
void app_shutdown(App *app);

void app_request_redraw(App *app);
void app_resize(App *app, int width, int height);

/* Dispatch a button press to the calculator engine. */
void app_press_button(App *app, int index);

/* Rebuild display_buffer / pending_buffer from calculator state. */
void app_update_display_buffers(App *app);

/* Font size controls (Ctrl+Plus / Ctrl+Minus / Ctrl+0). */
void app_change_font_scale(App *app, int delta_steps);
void app_reset_font_scale(App *app);
void app_toggle_theme(App *app);

#endif
