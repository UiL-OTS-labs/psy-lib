
#include <glib.h>

#include "cmd_vsync_opts.h"

static CmdVSyncOptions g_options = {
    .isi_dur   = .250,
    .stim_dur  = .250,
    .num_stims = 10,
};

static bool g_parsed = false;

// clang-format off
static GOptionEntry entries[] = {
    {"monitor", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_options.nth_monitor,
        "Put the display on the nth monitor", "nth"},
    {"fullscreen", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_options.fullscreen,
        "Create a full screen display(or not)", NULL},
    {"dur", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_options.stim_dur,
        "The duration of a single rectangle", "Seconds"},
    {"isi", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_options.isi_dur,
        "The duration between two successive rectangles", "Seconds"},
    {"num-stims", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_options.num_stims,
        "The number of rectangles to present", "N"},
    {0}
};

// clang-format on

static void
validate_options(void)
{
    if (g_options.num_stims < 0) {
        g_options.num_stims = 10;
        g_warning("Invalid number of stims specified, presenting %d",
                  g_options.num_stims);
    }
    if (g_options.isi_dur < 0) {
        g_options.isi_dur = .250;
        g_warning("Invalid isi specified, isi is set to %lf",
                  g_options.isi_dur);
    }
    if (g_options.stim_dur < 0) {
        g_options.stim_dur = .250;
        g_warning("Invalid stimulus duration specified, duration is set to %lf",
                  g_options.stim_dur);
    }
}

bool
cmd_vsync_parse(int *argc, char ***argv)
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

    // check whether the chosen options are sane fix them if applicable.
    validate_options();

parse_error:
    g_option_context_free(context);
no_context:
    g_parsed = status;
    return status;
}

const CmdVSyncOptions *
cmd_vsync_get_options(void)
{
    if (g_parsed) {
        return &g_options;
    }
    return NULL;
}
