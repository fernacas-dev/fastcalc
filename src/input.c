#include "input.h"

#include "clipboard.h"

static void state_changed(App *app)
{
    app_update_display_buffers(app);
    app_request_redraw(app);
}

/* Printable characters arrive here, not via keydown. SDL text input yields
 * the real character for any layout: Shift+'=' gives '+', Shift+'8' gives
 * '*', keypad digits give '0'-'9' with NumLock on, etc. */
static void handle_text(App *app, const char *text)
{
    if (text == NULL) {
        return;
    }
    bool changed = false;
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        unsigned char c = *p;
        if (c >= (unsigned char)'0' && c <= (unsigned char)'9') {
            calc_input_digit(&app->calc, (int)(c - (unsigned char)'0'));
            changed = true;
            continue;
        }
        switch (c) {
        case (unsigned char)'+':
            calc_set_operator(&app->calc, OP_ADD);
            changed = true;
            break;
        case (unsigned char)'-':
            calc_set_operator(&app->calc, OP_SUBTRACT);
            changed = true;
            break;
        case (unsigned char)'*':
            calc_set_operator(&app->calc, OP_MULTIPLY);
            changed = true;
            break;
        case (unsigned char)'/':
            calc_set_operator(&app->calc, OP_DIVIDE);
            changed = true;
            break;
        case (unsigned char)'=':
            calc_equals(&app->calc);
            changed = true;
            break;
        case (unsigned char)'.':
        case (unsigned char)',':
            calc_input_decimal(&app->calc);
            changed = true;
            break;
        case (unsigned char)'%':
            calc_percent(&app->calc);
            changed = true;
            break;
        case (unsigned char)'c':
        case (unsigned char)'C':
            calc_clear(&app->calc);
            changed = true;
            break;
        default:
            break;
        }
    }
    if (changed) {
        state_changed(app);
    }
}

static void handle_key_down(App *app, const SDL_KeyboardEvent *key)
{
    bool ctrl = (key->mod & SDL_KMOD_CTRL) != 0;
    SDL_Keycode code = key->key;

    if (ctrl) {
        switch (code) {
        case SDLK_PLUS:
        case SDLK_EQUALS:
        case SDLK_KP_PLUS:
            app_change_font_scale(app, 1);
            return;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            app_change_font_scale(app, -1);
            return;
        case SDLK_0:
        case SDLK_KP_0:
            app_reset_font_scale(app);
            return;
        case SDLK_C:
            clipboard_copy(app);
            return;
        case SDLK_V:
            clipboard_paste(app);
            return;
        case SDLK_T:
        case SDLK_D:
            app_toggle_theme(app);
            return;
        default:
            return; /* Ignore other Ctrl combinations. */
        }
    }

    if (app->text_input_active) {
        /* Printable main-keyboard keys are handled via SDL_EVENT_TEXT_INPUT
         * (layout-correct); acting on them here would double-apply. */
        if (code >= SDLK_SPACE && code <= 0x7Eu) {
            return;
        }
        /* Keypad digits/point arrive as text with NumLock on; without
         * NumLock they are navigation keys and do nothing here. */
        switch (code) {
        case SDLK_KP_0:
        case SDLK_KP_1:
        case SDLK_KP_2:
        case SDLK_KP_3:
        case SDLK_KP_4:
        case SDLK_KP_5:
        case SDLK_KP_6:
        case SDLK_KP_7:
        case SDLK_KP_8:
        case SDLK_KP_9:
        case SDLK_KP_PERIOD:
        case SDLK_KP_COMMA:
            return;
        default:
            break;
        }
    }

    /* Control keys, keypad operators (which produce no text), and the
     * fallback path when text input is unavailable. */
    switch (code) {
    case SDLK_0:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9:
        calc_input_digit(&app->calc, (int)(code - SDLK_0));
        state_changed(app);
        break;
    case SDLK_KP_0:
    case SDLK_KP_1:
    case SDLK_KP_2:
    case SDLK_KP_3:
    case SDLK_KP_4:
    case SDLK_KP_5:
    case SDLK_KP_6:
    case SDLK_KP_7:
    case SDLK_KP_8:
    case SDLK_KP_9:
        calc_input_digit(&app->calc, (int)(code - SDLK_KP_0));
        state_changed(app);
        break;
    case SDLK_PLUS:
    case SDLK_KP_PLUS:
        calc_set_operator(&app->calc, OP_ADD);
        state_changed(app);
        break;
    case SDLK_MINUS:
    case SDLK_KP_MINUS:
        calc_set_operator(&app->calc, OP_SUBTRACT);
        state_changed(app);
        break;
    case SDLK_ASTERISK:
    case SDLK_KP_MULTIPLY:
        calc_set_operator(&app->calc, OP_MULTIPLY);
        state_changed(app);
        break;
    case SDLK_SLASH:
    case SDLK_KP_DIVIDE:
        calc_set_operator(&app->calc, OP_DIVIDE);
        state_changed(app);
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
    case SDLK_EQUALS:
    case SDLK_KP_EQUALS:
        calc_equals(&app->calc);
        state_changed(app);
        break;
    case SDLK_PERIOD:
    case SDLK_KP_PERIOD:
    case SDLK_COMMA:
    case SDLK_KP_COMMA:
        calc_input_decimal(&app->calc);
        state_changed(app);
        break;
    case SDLK_BACKSPACE:
    case SDLK_KP_BACKSPACE:
        calc_backspace(&app->calc);
        state_changed(app);
        break;
    case SDLK_ESCAPE:
    case SDLK_DELETE:
    case SDLK_C:
        calc_clear(&app->calc);
        state_changed(app);
        break;
    case SDLK_PERCENT:
        calc_percent(&app->calc);
        state_changed(app);
        break;
    case SDLK_F10:
        app_toggle_theme(app);
        break;
    default:
        break;
    }
}

static void handle_mouse_motion(App *app, const SDL_MouseMotionEvent *motion)
{
    int hit = layout_hit_test(app->buttons, motion->x, motion->y);
    if (hit != app->hover_button) {
        app->hover_button = hit;
        app_request_redraw(app);
    }
}

static void handle_button_down(App *app, const SDL_MouseButtonEvent *button)
{
    if (button->button != SDL_BUTTON_LEFT) {
        return;
    }
    int hit = layout_hit_test(app->buttons, button->x, button->y);
    app->pressed_button = hit;
    app_request_redraw(app);
}

static void handle_button_up(App *app, const SDL_MouseButtonEvent *button)
{
    if (button->button != SDL_BUTTON_LEFT) {
        return;
    }
    int was_pressed = app->pressed_button;
    app->pressed_button = -1;
    int hit = layout_hit_test(app->buttons, button->x, button->y);
    if (was_pressed >= 0 && was_pressed == hit) {
        app_press_button(app, hit);
    } else {
        app_request_redraw(app);
    }
}

void input_handle_event(App *app, const SDL_Event *event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        app->running = false;
        break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        app_resize(app, (int)event->window.data1, (int)event->window.data2);
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        app_request_redraw(app);
        break;
    case SDL_EVENT_KEY_DOWN:
        handle_key_down(app, &event->key);
        break;
    case SDL_EVENT_TEXT_INPUT:
        handle_text(app, event->text.text);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        handle_mouse_motion(app, &event->motion);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        handle_button_down(app, &event->button);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        handle_button_up(app, &event->button);
        break;
    default:
        break;
    }
}
