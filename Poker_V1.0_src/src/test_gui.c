#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(condition, message)                  \
    do {                                                 \
        tests_run++;                                     \
        if (!(condition)) {                              \
            tests_failed++;                              \
            printf("FAIL: %s\n", message);              \
        } else {                                         \
            printf("PASS: %s\n", message);              \
        }                                                \
    } while (0)

int main(void)
{
    SDL_Window *window;

    printf("Running GUI tests...\n");

    setenv("SDL_VIDEODRIVER", "dummy", 1);

    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO) == 0, "SDL video initializes with dummy driver");

    window = SDL_CreateWindow(
        "GUI Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        200,
        100,
        SDL_WINDOW_HIDDEN
    );

    ASSERT_TRUE(window != NULL, "hidden SDL window can be created");

    if (window != NULL) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();

    printf("GUI tests run: %d\n", tests_run);
    printf("GUI tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("GUI test passed.\n");
        return 0;
    }

    printf("GUI test failed.\n");
    return 1;
}