
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CmdOptions {
    int    nth_monitor;
    bool   fullscreen;
    double stim_dur;
    double isi_dur;
    int    num_stims;
} CmdOptions;

bool
cmd_parse(int *argc, char ***argv);

const CmdOptions *
cmd_get_options(void);

#ifdef __cplusplus
}
#endif
