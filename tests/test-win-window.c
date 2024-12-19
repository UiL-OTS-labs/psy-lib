
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <psylib.h>

int
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR     lpCmdLine,
        int       nShowCmd)
{
    psy_init();
    // g_log_set_fatal_mask("Psy", G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
    PsyWinWindow *win = psy_win_window_new();

    psy_win_window_start_message_loop(win); // returns when window is done

    psy_deinit();

    return 0;
}
