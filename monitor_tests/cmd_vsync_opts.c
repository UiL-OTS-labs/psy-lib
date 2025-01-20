
#include <glib.h>

#include "cmd_vsync_opts.h"

CmdOptions  g_options;
static bool g_parsed = false;

// clang-format off
GOptionEntry entries[] = {
    {"monitor", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_options.nth_monitor,
        "Put the display on the nth monitor", NULL},
    {"fullscreen", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_options.fullscreen,
        "Create a fullscreen display(or not)", NULL},
    {0}
};

// clang-format on

bool
cmd_parse(int *argc, char ***argv)
{
    GError         *error   = NULL;
    bool            status  = true;
    GOptionContext *context = g_option_context_new(NULL);

    if (!context) {
        status = false;
        goto no_context;
    }

    g_option_context_add_main_entries(context, entries, NULL);

    g_option_context_parse(context, argc, argv, &error);
    if (error) {
        g_critical("Unable to parse the options: %s", error->message);
        status = false;
        goto parse_error;
    }

parse_error:
    g_option_context_free(context);
no_context:
    g_parsed = status;
    return status;
}

const CmdOptions *
cmd_get_options(void)
{
    if (g_parsed) {
        return &g_options;
    }
    return NULL;
}
