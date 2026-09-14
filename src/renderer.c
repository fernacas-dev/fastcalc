#include "renderer.h"

static void set_color(SDL_Renderer *r, SDL_Color c)
{
    (void)SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

/* Flat hover tint: nudge toward white without gradients or shadows. */
static SDL_Color lighten(SDL_Color c)
{
    const int amount = 14;
    int r = (int)c.r + amount;
    int g = (int)c.g + amount;
    int b = (int)c.b + amount;
    c.r = (Uint8)(r > 255 ? 255 : r);
    c.g = (Uint8)(g > 255 ? 255 : g);
    c.b = (Uint8)(b > 255 ? 255 : b);
    return c;
}

/* Shrink the display font just enough to fit very long values. Long values
 * are rare, so re-rasterizing on those frames is acceptable. */
static void fit_display_font(App *app, float avail_width)
{
    int width = font_text_width(&app->display_font, app->display_buffer);
    int size = app->display_font.pixel_size;
    while ((float)width > avail_width && size > APP_FONT_MIN_PX) {
        size -= 4;
        if (size < APP_FONT_MIN_PX) {
            size = APP_FONT_MIN_PX;
        }
        (void)font_set_size(&app->display_font, size);
        width = font_text_width(&app->display_font, app->display_buffer);
        if (size == APP_FONT_MIN_PX) {
            break;
        }
    }
}

void renderer_draw(App *app)
{
    SDL_Renderer *r = app->renderer;
    const Theme *t = &app->theme;

    set_color(r, t->background);
    (void)SDL_RenderClear(r);

    /* Display panel. */
    set_color(r, t->display);
    (void)SDL_RenderFillRect(r, &app->display_rect);
    set_color(r, t->border);
    (void)SDL_RenderRect(r, &app->display_rect);

    const float pad = 12.0f;
    float inner_x = app->display_rect.x + pad;
    float inner_w = app->display_rect.w - 2.0f * pad;

    /* Pending line ("12.5 +") in the small font, main value large. */
    if (app->pending_buffer[0] != '\0') {
        font_draw_text(&app->button_font, app->pending_buffer, inner_x,
                       app->display_rect.y + 8.0f, inner_w, ALIGN_RIGHT, t->text);
    }

    fit_display_font(app, inner_w);

    SDL_Color value_color = t->text;
    if (app->calc.error) {
        value_color.r = 255;
        value_color.g = 110;
        value_color.b = 110;
        value_color.a = 255;
        if (!app->dark_mode) {
            value_color.r = 190;
            value_color.g = 30;
            value_color.b = 30;
        }
    }
    float main_y = app->display_rect.y + app->display_rect.h -
                   (float)app->display_font.pixel_size - 12.0f;
    if (main_y < app->display_rect.y + 8.0f) {
        main_y = app->display_rect.y + 8.0f;
    }
    font_draw_text(&app->display_font, app->display_buffer, inner_x, main_y,
                   inner_w, ALIGN_RIGHT, value_color);

    /* Buttons. */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        const Button *b = &app->buttons[i];
        bool is_op = (b->type == BUTTON_OPERATOR || b->type == BUTTON_EQUALS);
        SDL_Color fill = is_op ? t->operator : t->button;
        if (i == app->pressed_button) {
            fill = t->border;
        } else if (i == app->hover_button) {
            fill = lighten(fill);
        }
        set_color(r, fill);
        (void)SDL_RenderFillRect(r, &b->rect);
        set_color(r, t->border);
        (void)SDL_RenderRect(r, &b->rect);

        float label_y = b->rect.y + (b->rect.h - (float)app->button_font.pixel_size) * 0.5f;
        font_draw_text(&app->button_font, b->label, b->rect.x, label_y, b->rect.w,
                       ALIGN_CENTER, t->button_text);
    }

    (void)SDL_RenderPresent(r);
}
