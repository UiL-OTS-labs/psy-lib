
#include <psylib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>

#include <stdlib.h>

#include "cmd_enum_opts.h"

// Notice, monitors are called SDL_Displays
//
static void
enumerate_modes(SDL_DisplayID display)
{
    SDL_DisplayMode **modes = NULL;
    int               count;

    modes = SDL_GetFullscreenDisplayModes(display, &count);
    if (!modes) {
        g_critical("Unable to get display modes: %s", SDL_GetError());
        return;
    }

    for (int i = 0; i < count; i++) {
        SDL_DisplayMode *mode = modes[i];
        g_print("        w =%5d, h =%5d, %d/%d %fHz, format=%s\n",
                mode->w,
                mode->h,
                mode->refresh_rate_numerator,
                mode->refresh_rate_denominator,
                mode->refresh_rate,
                SDL_GetPixelFormatName(mode->format));
    }

    SDL_free(modes);
}

void
enumerate_displays(void)
{
    int            num_displays = 0;
    SDL_DisplayID *ids          = SDL_GetDisplays(&num_displays);
    SDL_Rect       r;

    const CmdOptions *opts = cmd_get_options();

    for (int i = 0; i < num_displays; i++) {

        g_print("index %d - name:'%s'\n", i, SDL_GetDisplayName(ids[i]));
        if (SDL_GetDisplayBounds(ids[i], &r)) {
            g_print(
                "    x=%-6dy=%-6dwidth=%-6dheight=%-6d\n", r.x, r.y, r.w, r.h);
        }

        if (opts->show_modes) {
            enumerate_modes(ids[i]);
        }
    }
}

int
main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;

    if (!cmd_parse(&argc, &argv)) {
        status = EXIT_FAILURE;
        goto init_error;
    }
    if (!SDL_Init(SDL_Init(SDL_INIT_VIDEO))) {
        SDL_Log("Unable to init sdl3: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto init_error;
    }

    enumerate_displays();

    SDL_Quit();
init_error:
    return status;
}
