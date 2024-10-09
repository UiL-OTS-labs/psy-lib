
// This toy tests whether sleeping is affected by the windows function 
// timeBeginPeriod. When you set the resolution to a low value,
// The system might act more responsive on the expense of
// using more power.

#include <psylib.h>

#if _WIN32
#include <windows.h>
#endif

#include <unistd.h>

static guint time_resolution = 0;
static const char* sleep_func = NULL;

// clang-format off
GOptionEntry options[] = {
    {"begin-period", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &time_resolution, "(windows only)Specify a value for timeBeginPeriod [0-20] 0 == off", "N"},
    {"sleep-method", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &sleep_func, "specify the sleep func {Sleep, g_usleep, usleep}", "Method"},
    {0}
};
// clang-format on

int main(int argc, char**argv) {

    GError* error = NULL;

    GOptionContext* opts = g_option_context_new("sleepy time options");
    g_option_context_add_main_entries(opts, options, NULL);

    if (!g_option_context_parse(opts, &argc, &argv, &error)) {
        g_printerr("Oops unable to parse options: %s\n", error->message);
        g_option_context_free(opts);
        return EXIT_FAILURE;
    }
    
    PsyClock* clk = psy_clock_new();

#if _WIN32 
    if (time_resolution) {
        int ret = timeBeginPeriod(time_resolution);
        if (ret == TIMERR_NOCANDO) {
            g_printerr("Oops invalid value for timeBeginPeriod.\n");
            return 1;
        }
    }
#endif

    PsyTimePoint* tp1 = psy_clock_now(clk);

    if (g_strcmp0(sleep_func, "g_usleep") == 0)
        g_usleep(1000);
    else if (g_strcmp0(sleep_func, "usleep") == 0 )
        usleep(1000);
    else if  (g_strcmp0(sleep_func, "Sleep") == 0)
        Sleep(1);
    else
        g_usleep(1000);

    PsyTimePoint* tp2 = psy_clock_now(clk);

#if _WIN32
    if (time_resolution) {
        timeEndPeriod(time_resolution);
    }
#endif

    PsyDuration* dur = psy_time_point_subtract(tp2, tp1);

    g_print("Sleeping lasted for %lf seconds", psy_duration_get_seconds(dur));

    psy_duration_free(dur);
    psy_time_point_free(tp2);
    psy_time_point_free(tp1);
    psy_clock_free(clk);

}