#include "calculator.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double apply_operator(Operator op, double lhs, double rhs, bool *error)
{
    double result = 0.0;

    switch (op) {
    case OP_NONE:
        result = rhs;
        break;
    case OP_ADD:
        result = lhs + rhs;
        break;
    case OP_SUBTRACT:
        result = lhs - rhs;
        break;
    case OP_MULTIPLY:
        result = lhs * rhs;
        break;
    case OP_DIVIDE:
        if (rhs == 0.0) {
            *error = true;
            return 0.0;
        }
        result = lhs / rhs;
        break;
    }

    if (!isfinite(result)) {
        *error = true;
        return 0.0;
    }
    return result;
}

/* Parse the in-progress keystrokes. Empty / lone "-" / "0." prefixes
 * strtod handles natively; treat parse failure as 0.0. */
static double parse_input(const CalculatorState *s)
{
    if (!s->entering || s->input[0] == '\0') {
        return 0.0;
    }
    return strtod(s->input, NULL);
}

void calc_init(CalculatorState *s)
{
    calc_clear(s);
}

void calc_clear(CalculatorState *s)
{
    s->accumulator = 0.0;
    s->pending_op = OP_NONE;
    s->input[0] = '\0';
    s->entering = false;
    s->error = false;
    s->just_evaluated = false;
}

void calc_input_digit(CalculatorState *s, int digit)
{
    if (digit < 0 || digit > 9) {
        return;
    }
    if (s->error) {
        calc_clear(s);
    }
    if (s->just_evaluated && !s->entering) {
        /* Fresh entry after '=': discard the old result. */
        s->input[0] = '\0';
        s->entering = true;
        s->just_evaluated = false;
    }
    if (!s->entering) {
        s->entering = true;
        s->input[0] = '\0';
    }

    size_t len = strlen(s->input);
    if (len == 1 && s->input[0] == '0' && strchr(s->input, '.') == NULL) {
        /* Avoid "007": replace a lone leading zero. */
        s->input[0] = (char)('0' + digit);
        return;
    }
    if (len == 2 && s->input[0] == '-' && s->input[1] == '0' &&
        strchr(s->input, '.') == NULL) {
        s->input[1] = (char)('0' + digit);
        return;
    }
    if (len + 1 >= CALC_INPUT_CAP) {
        return; /* Bounded buffer: ignore overflow keystrokes. */
    }
    s->input[len] = (char)('0' + digit);
    s->input[len + 1] = '\0';
}

void calc_input_decimal(CalculatorState *s)
{
    if (s->error) {
        calc_clear(s);
    }
    if (s->just_evaluated && !s->entering) {
        s->input[0] = '\0';
        s->entering = true;
        s->just_evaluated = false;
    }
    if (!s->entering) {
        s->entering = true;
        s->input[0] = '\0';
    }
    if (strchr(s->input, '.') != NULL) {
        return; /* Only one decimal point per entry. */
    }
    size_t len = strlen(s->input);
    if (len == 0) {
        (void)snprintf(s->input, sizeof(s->input), "0.");
    } else if (len == 1 && s->input[0] == '-') {
        (void)snprintf(s->input, sizeof(s->input), "-0.");
    } else {
        if (len + 1 >= CALC_INPUT_CAP) {
            return;
        }
        s->input[len] = '.';
        s->input[len + 1] = '\0';
    }
}

void calc_backspace(CalculatorState *s)
{
    if (s->error) {
        calc_clear(s);
        return;
    }
    if (!s->entering) {
        return;
    }
    size_t len = strlen(s->input);
    if (len == 0) {
        s->entering = false;
        return;
    }
    s->input[len - 1] = '\0';
    if (s->input[0] == '\0' || strcmp(s->input, "-") == 0) {
        s->input[0] = '\0';
        s->entering = false;
    }
}

void calc_negate(CalculatorState *s)
{
    if (s->error) {
        return;
    }
    if (s->entering && s->input[0] != '\0' && strcmp(s->input, "0") != 0 &&
        strcmp(s->input, "0.") != 0) {
        if (s->input[0] == '-') {
            size_t len = strlen(s->input);
            memmove(s->input, s->input + 1, len); /* Includes NUL. */
        } else {
            size_t len = strlen(s->input);
            if (len + 1 < CALC_INPUT_CAP) {
                memmove(s->input + 1, s->input, len + 1);
                s->input[0] = '-';
            }
        }
        return;
    }
    if (!s->entering) {
        double v = -s->accumulator;
        if (v == 0.0) {
            v = 0.0; /* Normalize -0. */
        }
        s->accumulator = v;
        s->just_evaluated = false;
    }
}

void calc_percent(CalculatorState *s)
{
    if (s->error) {
        return;
    }
    if (s->entering) {
        double entered = parse_input(s);
        double result;
        if (s->pending_op == OP_ADD || s->pending_op == OP_SUBTRACT) {
            /* 200 + 10 % -> 210 (percentage of the left operand). */
            result = s->accumulator * entered / 100.0;
        } else {
            /* Standalone or x//: plain hundredths. */
            result = entered / 100.0;
        }
        if (!isfinite(result)) {
            s->error = true;
            s->entering = false;
            return;
        }
        format_number(result, s->input, sizeof(s->input));
        s->entering = true;
    } else {
        double result = s->accumulator / 100.0;
        if (!isfinite(result)) {
            s->error = true;
            return;
        }
        s->accumulator = result;
    }
    s->just_evaluated = false;
}

void calc_set_operator(CalculatorState *s, Operator op)
{
    if (s->error || op == OP_NONE) {
        return;
    }
    if (s->entering) {
        double entered = parse_input(s);
        bool err = false;
        double result;
        if (s->pending_op == OP_NONE && !s->just_evaluated) {
            result = entered;
        } else if (s->pending_op == OP_NONE) {
            /* Chaining after '=': accumulator already holds the result. */
            result = s->accumulator;
            (void)entered;
        } else {
            result = apply_operator(s->pending_op, s->accumulator, entered, &err);
        }
        if (err) {
            s->error = true;
            s->entering = false;
            return;
        }
        s->accumulator = result;
        s->entering = false;
        s->input[0] = '\0';
    }
    /* Not entering: just (re)select the pending operator. */
    s->pending_op = op;
    s->just_evaluated = false;
}

void calc_equals(CalculatorState *s)
{
    if (s->error || s->pending_op == OP_NONE) {
        return;
    }
    double rhs = s->entering ? parse_input(s) : s->accumulator;
    bool err = false;
    double result = apply_operator(s->pending_op, s->accumulator, rhs, &err);
    if (err) {
        s->error = true;
        s->entering = false;
        return;
    }
    s->accumulator = result;
    s->pending_op = OP_NONE;
    s->entering = false;
    s->input[0] = '\0';
    s->just_evaluated = true;
}

double calc_display_value(const CalculatorState *s)
{
    if (s->error) {
        return 0.0;
    }
    if (s->entering) {
        return parse_input(s);
    }
    return s->accumulator;
}

void calc_display_text(const CalculatorState *s, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) {
        return;
    }
    if (s->error) {
        (void)snprintf(buffer, buffer_size, "Error");
        return;
    }
    if (s->entering) {
        if (s->input[0] == '\0') {
            (void)snprintf(buffer, buffer_size, "0");
        } else {
            (void)snprintf(buffer, buffer_size, "%s", s->input);
        }
        return;
    }
    format_number(s->accumulator, buffer, buffer_size);
}

bool calc_set_input_text(CalculatorState *s, const char *text)
{
    if (text == NULL || text[0] == '\0') {
        return false;
    }
    /* Strict plain-number validation: [+-]? digits [. digits]. */
    size_t i = 0;
    if (text[0] == '+' || text[0] == '-') {
        i = 1;
        if (text[i] == '\0') {
            return false;
        }
    }
    bool any_digit = false;
    bool seen_dot = false;
    for (; text[i] != '\0'; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            any_digit = true;
        } else if (c == '.' && !seen_dot) {
            seen_dot = true;
        } else {
            return false;
        }
    }
    if (!any_digit) {
        return false;
    }
    if (strlen(text) + 1 > CALC_INPUT_CAP) {
        return false;
    }
    if (s->error) {
        calc_clear(s);
    }
    (void)snprintf(s->input, sizeof(s->input), "%s", text);
    /* Normalize "+5" -> "5". */
    if (s->input[0] == '+') {
        size_t len = strlen(s->input);
        memmove(s->input, s->input + 1, len);
    }
    s->entering = true;
    s->just_evaluated = false;
    return true;
}

void format_number(double value, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) {
        return;
    }
    if (!isfinite(value)) {
        (void)snprintf(buffer, buffer_size, "Error");
        return;
    }
    if (value == 0.0) {
        (void)snprintf(buffer, buffer_size, "0");
        return;
    }
    (void)snprintf(buffer, buffer_size, "%.15g", value);
    if (strcmp(buffer, "-0") == 0) {
        (void)snprintf(buffer, buffer_size, "0");
    }
}

const char *operator_symbol(Operator op)
{
    switch (op) {
    case OP_ADD:
        return "+";
    case OP_SUBTRACT:
        return "-";
    case OP_MULTIPLY:
        return "x";
    case OP_DIVIDE:
        return "/";
    case OP_NONE:
        return "";
    }
    return "";
}
