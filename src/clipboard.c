#include "clipboard.h"

#include <string.h>

void clipboard_copy(App *app)
{
    char buffer[128];
    calc_display_text(&app->calc, buffer, sizeof(buffer));
    (void)SDL_SetClipboardText(buffer);
}

void clipboard_paste(App *app)
{
    char *raw = SDL_GetClipboardText();
    if (raw == NULL) {
        return;
    }

    /* Trim surrounding whitespace into a bounded local buffer. */
    char clean[128];
    size_t n = 0;
    for (const char *p = raw; *p != '\0' && n + 1 < sizeof(clean); p++) {
        char c = *p;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            clean[n++] = c;
        }
    }
    clean[n] = '\0';
    SDL_free(raw);

    if (n == 0) {
        return;
    }
    if (calc_set_input_text(&app->calc, clean)) {
        app_update_display_buffers(app);
        app_request_redraw(app);
    }
}
