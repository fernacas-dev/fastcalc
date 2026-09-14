#ifndef FASTCALC_CLIPBOARD_H
#define FASTCALC_CLIPBOARD_H

#include "app.h"

/* Copy the current display value (Ctrl+C). Never fatal on clipboard errors. */
void clipboard_copy(App *app);

/* Paste a plain numeric value (Ctrl+V). Invalid content is ignored. */
void clipboard_paste(App *app);

#endif
