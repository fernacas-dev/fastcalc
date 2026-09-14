#include "app.h"

#include <stdio.h>

#include "input.h"
#include "renderer.h"

/* System font candidates, first hit wins. DejaVu Sans ships with
 * virtually every Linux desktop distribution. */
static const char *FONT_CANDIDATES[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
};

static const char *find_font_path(void)
{
    for (size_t i = 0; i < sizeof(FONT_CANDIDATES) / sizeof(FONT_CANDIDATES[0]); i++) {
        SDL_IOStream *io = SDL_IOFromFile(FONT_CANDIDATES[i], "r");
        if (io != NULL) {
            SDL_CloseIO(io);
            return FONT_CANDIDATES[i];
        }
    }
    return NULL;
}

static int display_px_for_steps(int steps)
{
    int px = APP_DISPLAY_FONT_PX + steps * 4;
    if (px < APP_FONT_MIN_PX) {
        px = APP_FONT_MIN_PX;
    }
    if (px > APP_FONT_MAX_PX) {
        px = APP_FONT_MAX_PX;
    }
    return px;
}

static int button_px_for_steps(int steps)
{
    int px = APP_BUTTON_FONT_PX + steps * 2;
    if (px < 12) {
        px = 12;
    }
    if (px > 40) {
        px = 40;
    }
    return px;
}

bool app_init(App *app)
{
    app->window = NULL;
    app->renderer = NULL;
    app->running = false;
    app->needs_redraw = true;
    app->window_width = APP_INIT_WIDTH;
    app->window_height = APP_INIT_HEIGHT;
    app->hover_button = -1;
    app->pressed_button = -1;
    app->font_steps = 0;
    app->text_input_active = false;
    app->dark_mode = true;
    app->theme = theme_dark();
    app->display_font.loaded = false;
    app->button_font.loaded = false;
    calc_init(&app->calc);
    app->display_buffer[0] = '\0';
    app->pending_buffer[0] = '\0';

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        (void)fprintf(stderr, "fastcalc: SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow("FastCalc", APP_INIT_WIDTH, APP_INIT_HEIGHT,
                                   SDL_WINDOW_RESIZABLE);
    if (app->window == NULL) {
        (void)fprintf(stderr, "fastcalc: SDL_CreateWindow failed: %s\n",
                      SDL_GetError());
        SDL_Quit();
        return false;
    }
    SDL_SetWindowMinimumSize(app->window, APP_MIN_WIDTH, APP_MIN_HEIGHT);

    app->renderer = SDL_CreateRenderer(app->window, NULL);
    if (app->renderer == NULL) {
        (void)fprintf(stderr, "fastcalc: SDL_CreateRenderer failed: %s\n",
                      SDL_GetError());
        SDL_DestroyWindow(app->window);
        app->window = NULL;
        SDL_Quit();
        return false;
    }

    const char *font_path = find_font_path();
    if (font_path == NULL) {
        (void)fprintf(stderr, "fastcalc: no usable system font found\n");
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        app->renderer = NULL;
        app->window = NULL;
        SDL_Quit();
        return false;
    }

    if (!font_load(&app->display_font, app->renderer, font_path,
                   APP_DISPLAY_FONT_PX)) {
        (void)fprintf(stderr, "fastcalc: failed to load font %s\n", font_path);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        app->renderer = NULL;
        app->window = NULL;
        SDL_Quit();
        return false;
    }
    if (!font_load(&app->button_font, app->renderer, font_path,
                   APP_BUTTON_FONT_PX)) {
        (void)fprintf(stderr, "fastcalc: failed to load font %s\n", font_path);
        font_unload(&app->display_font);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        app->renderer = NULL;
        app->window = NULL;
        SDL_Quit();
        return false;
    }

    int w = APP_INIT_WIDTH;
    int h = APP_INIT_HEIGHT;
    (void)SDL_GetWindowSize(app->window, &w, &h);
    app_resize(app, w, h);
    app_update_display_buffers(app);
    /* Printable keys go through SDL text input so every layout yields the
     * real character ('+' for Shift+'=', keypad digits, ...). If this
     * fails, keydown handling still covers the US layout as a fallback. */
    app->text_input_active = SDL_StartTextInput(app->window);
    if (!app->text_input_active) {
        (void)fprintf(stderr, "fastcalc: text input unavailable: %s\n",
                      SDL_GetError());
    }

    app->running = true;
    return true;
}

void app_run(App *app)
{
    /* Initial paint before blocking on events. */
    renderer_draw(app);
    app->needs_redraw = false;

    while (app->running) {
        SDL_Event event;
        if (!SDL_WaitEvent(&event)) {
            continue;
        }
        /* Drain everything already queued so one state change renders once. */
        do {
            input_handle_event(app, &event);
            if (!app->running) {
                break;
            }
        } while (SDL_PollEvent(&event));

        if (app->needs_redraw && app->running) {
            renderer_draw(app);
            app->needs_redraw = false;
        }
    }
}

void app_shutdown(App *app)
{
    /* Reverse order of acquisition. */
    font_unload(&app->button_font);
    font_unload(&app->display_font);
    if (app->text_input_active) {
        (void)SDL_StopTextInput(app->window);
        app->text_input_active = false;
    }
    if (app->renderer != NULL) {
        SDL_DestroyRenderer(app->renderer);
        app->renderer = NULL;
    }
    if (app->window != NULL) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }
    SDL_Quit();
}

void app_request_redraw(App *app)
{
    app->needs_redraw = true;
}

void app_resize(App *app, int width, int height)
{
    if (width < APP_MIN_WIDTH) {
        width = APP_MIN_WIDTH;
    }
    if (height < APP_MIN_HEIGHT) {
        height = APP_MIN_HEIGHT;
    }
    app->window_width = width;
    app->window_height = height;
    layout_compute(app->buttons, width, height, &app->display_rect);
    app->hover_button = -1;
    app->pressed_button = -1;
    app_request_redraw(app);
}

void app_press_button(App *app, int index)
{
    if (index < 0 || index >= BUTTON_COUNT) {
        return;
    }
    const Button *b = &app->buttons[index];
    switch (b->type) {
    case BUTTON_NUMBER:
        calc_input_digit(&app->calc, b->value);
        break;
    case BUTTON_DECIMAL:
        calc_input_decimal(&app->calc);
        break;
    case BUTTON_OPERATOR:
        calc_set_operator(&app->calc, b->op);
        break;
    case BUTTON_EQUALS:
        calc_equals(&app->calc);
        break;
    case BUTTON_CLEAR:
        calc_clear(&app->calc);
        break;
    case BUTTON_PERCENT:
        calc_percent(&app->calc);
        break;
    case BUTTON_SIGN:
        calc_negate(&app->calc);
        break;
    case BUTTON_BACKSPACE:
        calc_backspace(&app->calc);
        break;
    }
    app_update_display_buffers(app);
    app_request_redraw(app);
}

void app_update_display_buffers(App *app)
{
    calc_display_text(&app->calc, app->display_buffer, sizeof(app->display_buffer));
    app->pending_buffer[0] = '\0';
    if (!app->calc.error && app->calc.pending_op != OP_NONE) {
        char acc[64];
        format_number(app->calc.accumulator, acc, sizeof(acc));
        (void)snprintf(app->pending_buffer, sizeof(app->pending_buffer), "%s %s",
                       acc, operator_symbol(app->calc.pending_op));
    }
}

static void apply_font_sizes(App *app)
{
    (void)font_set_size(&app->display_font, display_px_for_steps(app->font_steps));
    (void)font_set_size(&app->button_font, button_px_for_steps(app->font_steps));
}

void app_change_font_scale(App *app, int delta_steps)
{
    int next = app->font_steps + delta_steps;
    if (next < -6) {
        next = -6;
    }
    if (next > 7) {
        next = 7;
    }
    if (next == app->font_steps) {
        return;
    }
    app->font_steps = next;
    apply_font_sizes(app);
    app_request_redraw(app);
}

void app_reset_font_scale(App *app)
{
    if (app->font_steps == 0) {
        return;
    }
    app->font_steps = 0;
    apply_font_sizes(app);
    app_request_redraw(app);
}

void app_toggle_theme(App *app)
{
    app->dark_mode = !app->dark_mode;
    app->theme = app->dark_mode ? theme_dark() : theme_light();
    app_request_redraw(app);
}
