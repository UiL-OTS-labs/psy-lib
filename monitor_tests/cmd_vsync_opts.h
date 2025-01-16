
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CmdOptions {
    int nth_monitor;
} CmdOptions;

bool
cmd_parse(int *argc, char ***argv);

const CmdOptions *
cmd_get_options(void);

#ifdef __cplusplus
}
#endif
