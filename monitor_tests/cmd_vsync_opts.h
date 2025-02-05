
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CmdVSyncOptions {
    int    nth_monitor;
    bool   fullscreen;
    double stim_dur;
    double isi_dur;
    int    num_stims;
    int    port_num;
} CmdVSyncOptions;

bool
cmd_vsync_parse(int *argc, char ***argv);

const CmdVSyncOptions *
cmd_vsync_get_options(void);

#ifdef __cplusplus
}
#endif
