
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CmdOptions {
    bool show_modes;
} CmdOptions;


bool
cmd_parse(int *argc, char ***argv);

const CmdOptions *
cmd_get_options(void);

#ifdef __cplusplus
}
#endif
