#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "calculator.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        checks++;                                                              \
        if (!(cond)) {                                                         \
            failures++;                                                        \
            (void)printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
        }                                                                      \
    } while (0)

#define CHECK_TEXT(state, expected)                                            \
    do {                                                                       \
        char buf[128];                                                         \
        calc_display_text(&(state), buf, sizeof(buf));                         \
        checks++;                                                              \
        if (strcmp(buf, (expected)) != 0) {                                    \
            failures++;                                                        \
            (void)printf("FAIL %s:%d: got \"%s\", want \"%s\"\n", __FILE__,    \
                         __LINE__, buf, (expected));                           \
        }                                                                      \
    } while (0)

static void type_number(CalculatorState *s, const char *digits)
{
    for (const char *p = digits; *p != '\0'; p++) {
        if (*p == '.') {
            calc_input_decimal(s);
        } else {
            calc_input_digit(s, *p - '0');
        }
    }
}

static void test_basic_operations(void)
{
    CalculatorState s;

    calc_init(&s);
    type_number(&s, "2");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "2");
    calc_equals(&s);
    CHECK_TEXT(s, "4");

    calc_init(&s);
    type_number(&s, "10");
    calc_set_operator(&s, OP_SUBTRACT);
    type_number(&s, "3");
    calc_equals(&s);
    CHECK_TEXT(s, "7");

    calc_init(&s);
    type_number(&s, "5");
    calc_set_operator(&s, OP_MULTIPLY);
    type_number(&s, "5");
    calc_equals(&s);
    CHECK_TEXT(s, "25");

    calc_init(&s);
    type_number(&s, "20");
    calc_set_operator(&s, OP_DIVIDE);
    type_number(&s, "4");
    calc_equals(&s);
    CHECK_TEXT(s, "5");
}

static void test_floating_point_formatting(void)
{
    CalculatorState s;

    calc_init(&s);
    type_number(&s, "0.1");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "0.2");
    calc_equals(&s);
    CHECK_TEXT(s, "0.3"); /* Not 0.30000000000000004. */

    char buf[128];
    format_number(1.0, buf, sizeof(buf));
    CHECK(strcmp(buf, "1") == 0);
    format_number(1.5, buf, sizeof(buf));
    CHECK(strcmp(buf, "1.5") == 0);
    format_number(1.0, buf, sizeof(buf));
    CHECK(strcmp(buf, "1") == 0);
    format_number(-0.0, buf, sizeof(buf));
    CHECK(strcmp(buf, "0") == 0);
    format_number(0.30000000000000004, buf, sizeof(buf));
    CHECK(strcmp(buf, "0.3") == 0);
    format_number(INFINITY, buf, sizeof(buf));
    CHECK(strcmp(buf, "Error") == 0);
    format_number(NAN, buf, sizeof(buf));
    CHECK(strcmp(buf, "Error") == 0);
}

static void test_chained_operations(void)
{
    CalculatorState s;

    calc_init(&s);
    type_number(&s, "2");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "3");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "4");
    calc_equals(&s);
    CHECK_TEXT(s, "9");

    calc_init(&s);
    type_number(&s, "10");
    calc_set_operator(&s, OP_MULTIPLY);
    type_number(&s, "2");
    calc_set_operator(&s, OP_MULTIPLY);
    type_number(&s, "5");
    calc_equals(&s);
    CHECK_TEXT(s, "100");
}

static void test_sign_and_percent(void)
{
    CalculatorState s;

    calc_init(&s);
    type_number(&s, "5");
    calc_negate(&s);
    CHECK_TEXT(s, "-5");
    calc_negate(&s);
    CHECK_TEXT(s, "5");

    /* No malformed "--5" states. */
    calc_init(&s);
    type_number(&s, "5");
    calc_negate(&s);
    calc_negate(&s);
    CHECK_TEXT(s, "5");

    calc_init(&s);
    type_number(&s, "50");
    calc_percent(&s);
    CHECK_TEXT(s, "0.5");

    /* Percentage of the left operand for + / -. */
    calc_init(&s);
    type_number(&s, "200");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "10");
    calc_percent(&s);
    calc_equals(&s);
    CHECK_TEXT(s, "220");

    /* -5 + 10 = 5 ; 5 x -2 = -10. */
    calc_init(&s);
    type_number(&s, "5");
    calc_negate(&s);
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "10");
    calc_equals(&s);
    CHECK_TEXT(s, "5");

    calc_init(&s);
    type_number(&s, "5");
    calc_set_operator(&s, OP_MULTIPLY);
    type_number(&s, "2");
    calc_negate(&s);
    calc_equals(&s);
    CHECK_TEXT(s, "-10");
}

static void test_division_by_zero(void)
{
    CalculatorState s;

    calc_init(&s);
    type_number(&s, "10");
    calc_set_operator(&s, OP_DIVIDE);
    type_number(&s, "0");
    calc_equals(&s);
    CHECK_TEXT(s, "Error");

    /* Clear recovers. */
    calc_clear(&s);
    CHECK_TEXT(s, "0");

    /* Numeric input also recovers. */
    calc_init(&s);
    type_number(&s, "10");
    calc_set_operator(&s, OP_DIVIDE);
    type_number(&s, "0");
    calc_equals(&s);
    CHECK_TEXT(s, "Error");
    calc_input_digit(&s, 7);
    CHECK_TEXT(s, "7");
}

static void test_invalid_states(void)
{
    CalculatorState s;

    /* Second decimal point ignored. */
    calc_init(&s);
    type_number(&s, "1.2.5");
    CHECK_TEXT(s, "1.25");

    /* Leading '.' means "0.". */
    calc_init(&s);
    calc_input_decimal(&s);
    type_number(&s, "5");
    CHECK_TEXT(s, "0.5");

    /* Repeated operators: last one wins. */
    calc_init(&s);
    type_number(&s, "8");
    calc_set_operator(&s, OP_ADD);
    calc_set_operator(&s, OP_MULTIPLY);
    type_number(&s, "2");
    calc_equals(&s);
    CHECK_TEXT(s, "16");

    /* Repeated equals is stable. */
    calc_init(&s);
    type_number(&s, "2");
    calc_set_operator(&s, OP_ADD);
    type_number(&s, "2");
    calc_equals(&s);
    CHECK_TEXT(s, "4");
    calc_equals(&s);
    CHECK_TEXT(s, "4");

    /* Backspace. */
    calc_init(&s);
    type_number(&s, "123");
    calc_backspace(&s);
    CHECK_TEXT(s, "12");

    /* Negative zero displays as 0. */
    calc_init(&s);
    type_number(&s, "0");
    calc_negate(&s);
    CHECK_TEXT(s, "0");

    /* Overflow to infinity becomes Error, never inf/nan text. */
    calc_init(&s);
    type_number(&s, "1");
    for (int i = 0; i < 400; i++) {
        calc_set_operator(&s, OP_MULTIPLY);
        type_number(&s, "10");
        calc_equals(&s);
        if (s.error) {
            break;
        }
    }
    CHECK_TEXT(s, "Error");

    /* Very small nonzero values stay bounded and finite. */
    char tiny[128];
    format_number(DBL_MIN, tiny, sizeof(tiny));
    CHECK(tiny[0] != '\0' && strlen(tiny) < sizeof(tiny));

    /* Paste validation. */
    calc_init(&s);
    CHECK(calc_set_input_text(&s, "123.45") == true);
    CHECK_TEXT(s, "123.45");
    CHECK(calc_set_input_text(&s, "12a3") == false);
    CHECK(calc_set_input_text(&s, "") == false);
    CHECK(calc_set_input_text(&s, "--5") == false);
    CHECK_TEXT(s, "123.45"); /* Failed paste leaves state untouched. */
}

int main(void)
{
    test_basic_operations();
    test_floating_point_formatting();
    test_chained_operations();
    test_sign_and_percent();
    test_division_by_zero();
    test_invalid_states();

    (void)printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
