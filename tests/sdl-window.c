
#include "SDL_events.h"
#include "SDL_video.h"
#include "psy-duration.h"
#include "psy-time-point.h"
#include <SDL2/_real_SDL_config.h>
#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_error.h>
#include <epoxy/gl_generated.h>
#include <psy-clock.h>
#include <inttypes.h>

int main(int argc, char const * argv[argc-1]) {

    SDL_Init(SDL_INIT_VIDEO);

    PsyClock* clk = psy_clock_new();
    PsyTimePoint* start_time = psy_clock_now(clk);
    const int r=8,  g=8,  b=8; // RGB 
    int       or,  og,   ob; // and obtained RGB

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, r);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, b);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* win = SDL_CreateWindow (
            "SDL2 Window",
            0, 0,
            0, 0,
            SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
            );

    if (!win) {
        fprintf(stderr, "Unable to create an window: %s\n", SDL_GetError());
        goto premature_exit;
    }
    
    SDL_GLContext *glcontext = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, glcontext);
    
    SDL_GL_SetSwapInterval(1);

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &or);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &og);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &ob);

    fprintf(stdout, "We asked for r=%d g=%d b=%d\n", r, g, b);
    fprintf(stdout, "We got r=%d g=%d b=%d\n", or, og, ob);

    bool running = 1;

    while (running) {
        SDL_Event event;
        GLfloat r, g, b;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEMOTION:
                    break;
                default:
                    break;
                    //int a = 1;
                    //fprintf(stderr, "Oops unhandeled event\n");
            }
        }

        PsyTimePoint* t0, *t1, *t2;
        PsyDuration* t_since_start;
        t0 =  psy_clock_now(clk);
        t_since_start = psy_time_point_subtract(t0, start_time);
        double seconds = psy_duration_get_seconds(t_since_start);
        const double pi = 3.141592654;
        r = (GLfloat) sin(seconds * 2 * pi) / 2.0 + .5;
        g = (GLfloat) sin(seconds * 2 * pi) / 2.0 + .5;
        b = (GLfloat) sin(seconds * 2 * pi) / 2.0 + .5;

        glClearColor(r,g,b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        t1 = psy_clock_now(clk);
        SDL_GL_SwapWindow(win);
        t2 = psy_clock_now(clk);

        PsyDuration* d0, * d1;
        d0 = psy_time_point_subtract(t1, t0);
        d1 = psy_time_point_subtract(t2, t1);
        fprintf(stdout, "Drawing takes %08"PRId64" and swapping takes %08"PRId64"us\n",
                psy_duration_get_us(d0), psy_duration_get_us(d1)
                );
        g_object_unref(t0);
        g_object_unref(t1);
        g_object_unref(t2);
        g_object_unref(d0);
        g_object_unref(d1);
    }

    g_object_unref(clk);

premature_exit:
    SDL_Quit();
}
