#include "layout.h"

/* Row-major grid definition. Buttons are data, not code. */
typedef struct {
    const char *label;
    ButtonType type;
    int value;
    Operator op;
} ButtonDef;

static const ButtonDef GRID[BUTTON_COUNT] = {
    /* Row 0 */
    {"C", BUTTON_CLEAR, 0, OP_NONE},
    {"+/-", BUTTON_SIGN, 0, OP_NONE},
    {"%", BUTTON_PERCENT, 0, OP_NONE},
    {"/", BUTTON_OPERATOR, 0, OP_DIVIDE},
    /* Row 1 */
    {"7", BUTTON_NUMBER, 7, OP_NONE},
    {"8", BUTTON_NUMBER, 8, OP_NONE},
    {"9", BUTTON_NUMBER, 9, OP_NONE},
    {"x", BUTTON_OPERATOR, 0, OP_MULTIPLY},
    /* Row 2 */
    {"4", BUTTON_NUMBER, 4, OP_NONE},
    {"5", BUTTON_NUMBER, 5, OP_NONE},
    {"6", BUTTON_NUMBER, 6, OP_NONE},
    {"-", BUTTON_OPERATOR, 0, OP_SUBTRACT},
    /* Row 3 */
    {"1", BUTTON_NUMBER, 1, OP_NONE},
    {"2", BUTTON_NUMBER, 2, OP_NONE},
    {"3", BUTTON_NUMBER, 3, OP_NONE},
    {"+", BUTTON_OPERATOR, 0, OP_ADD},
    /* Row 4 */
    {"0", BUTTON_NUMBER, 0, OP_NONE},
    {".", BUTTON_DECIMAL, 0, OP_NONE},
    {"DEL", BUTTON_BACKSPACE, 0, OP_NONE},
    {"=", BUTTON_EQUALS, 0, OP_NONE},
};

void layout_compute(Button *buttons, int width, int height, SDL_FRect *display_rect)
{
    float w = (float)width;
    float h = (float)height;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].label = GRID[i].label;
        buttons[i].type = GRID[i].type;
        buttons[i].value = GRID[i].value;
        buttons[i].op = GRID[i].op;
    }

    /* Display takes roughly the top 28% of the window. */
    float display_h = h * 0.28f;
    if (display_h < 110.0f) {
        display_h = 110.0f;
    }
    if (display_h > h * 0.4f) {
        display_h = h * 0.4f;
    }

    display_rect->x = LAYOUT_PADDING;
    display_rect->y = LAYOUT_PADDING;
    display_rect->w = w - 2.0f * LAYOUT_PADDING;
    display_rect->h = display_h;

    float grid_y = LAYOUT_PADDING + display_h + LAYOUT_GAP;
    float grid_h = h - grid_y - LAYOUT_PADDING;
    if (grid_h < 0.0f) {
        grid_h = 0.0f;
    }

    float cell_w = (w - 2.0f * LAYOUT_PADDING - (float)(BUTTON_COLS - 1) * LAYOUT_GAP) /
                   (float)BUTTON_COLS;
    float cell_h = (grid_h - (float)(BUTTON_ROWS - 1) * LAYOUT_GAP) / (float)BUTTON_ROWS;

    for (int row = 0; row < BUTTON_ROWS; row++) {
        for (int col = 0; col < BUTTON_COLS; col++) {
            int i = row * BUTTON_COLS + col;
            buttons[i].rect.x = LAYOUT_PADDING + (float)col * (cell_w + LAYOUT_GAP);
            buttons[i].rect.y = grid_y + (float)row * (cell_h + LAYOUT_GAP);
            buttons[i].rect.w = cell_w;
            buttons[i].rect.h = cell_h;
        }
    }
}

int layout_hit_test(const Button *buttons, float x, float y)
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        const SDL_FRect *r = &buttons[i].rect;
        if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) {
            return i;
        }
    }
    return -1;
}
