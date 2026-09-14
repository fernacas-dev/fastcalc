#ifndef FASTCALC_INPUT_H
#define FASTCALC_INPUT_H

#include "app.h"

/* Route one SDL event: window, keyboard, mouse. Updates calculator state
 * and marks the app for redraw when something changed. */
void input_handle_event(App *app, const SDL_Event *event);

#endif
