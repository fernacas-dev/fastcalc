#include <stdlib.h>

#include "app.h"

int main(void)
{
    App app;
    if (!app_init(&app)) {
        return EXIT_FAILURE;
    }
    app_run(&app);
    app_shutdown(&app);
    return EXIT_SUCCESS;
}
