
#include <glib.h>

#include "cmd_enum_opts.h"

CmdEnumOptions g_options;
static bool    g_parsed = false;

// clang-format off
GOptionEntry entries[] = {
    {"show_modes", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_options.show_modes, "Display the modes of the monitor", NULL},
    {0}
};

// clang-format on

bool
cmd_enum_parse(int *argc, char ***argv)
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

const CmdEnumOptions *
cmd_enum_get_options(void)
{
    if (g_parsed) {
        return &g_options;
    }
    return NULL;
}
