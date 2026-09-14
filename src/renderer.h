#ifndef FASTCALC_RENDERER_H
#define FASTCALC_RENDERER_H

#include "app.h"

/* Single deterministic paint: background -> display -> buttons -> text.
 * Called only when state changed (event-driven rendering, no idle redraws). */
void renderer_draw(App *app);

#endif
