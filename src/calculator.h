#ifndef FASTCALC_CALCULATOR_H
#define FASTCALC_CALCULATOR_H

#include <stdbool.h>
#include <stddef.h>

/* Four-function operator model. No expression parsing in V1. */
typedef enum {
    OP_NONE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE
} Operator;

/* Raw keystroke buffer size (including NUL). */
#define CALC_INPUT_CAP 64

typedef struct {
    double accumulator;      /* Committed value / running result. */
    Operator pending_op;     /* Operator waiting for its right operand. */
    char input[CALC_INPUT_CAP]; /* Exact user keystrokes while entering a number. */
    bool entering;           /* True while the user is typing a number. */
    bool error;              /* Sticky error state (e.g. division by zero). */
    bool just_evaluated;     /* True right after '=' produced a result. */
} CalculatorState;

void calc_init(CalculatorState *s);
void calc_clear(CalculatorState *s);
void calc_input_digit(CalculatorState *s, int digit);
void calc_input_decimal(CalculatorState *s);
void calc_backspace(CalculatorState *s);
void calc_negate(CalculatorState *s);
void calc_percent(CalculatorState *s);
void calc_set_operator(CalculatorState *s, Operator op);
void calc_equals(CalculatorState *s);

/* Value currently shown on the display. Returns 0.0 when in error state. */
double calc_display_value(const CalculatorState *s);

/* NUL-terminated display text: "Error", the in-progress keystrokes, or the
 * formatted accumulator. Always bounded by buffer_size. */
void calc_display_text(const CalculatorState *s, char *buffer, size_t buffer_size);

/* Validate + install pasted text as the current entry. Returns false and
 * leaves state untouched when the text is not a plain number. */
bool calc_set_input_text(CalculatorState *s, const char *text);

/*
 * Number formatting policy (documented here, implemented once):
 *  - snprintf "%.15g": removes trailing zeros, no FP artifacts such as
 *    0.30000000000000004 for 0.1 + 0.2, bounded output.
 *  - Negative zero prints as "0".
 *  - Non-finite values print as "Error".
 *  - Very large/small magnitudes fall back to %g scientific notation
 *    (e.g. "1e+16") rather than overflowing the buffer.
 */
void format_number(double value, char *buffer, size_t buffer_size);

/* UTF-8 symbol for the pending-operator line ("+", "-", "x", "/"). */
const char *operator_symbol(Operator op);

#endif
