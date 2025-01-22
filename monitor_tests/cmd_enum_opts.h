
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CmdEnumOptions {
    bool show_modes;
} CmdEnumOptions;

bool
cmd_enum_parse(int *argc, char ***argv);

const CmdEnumOptions *
cmd_enum_get_options(void);

#ifdef __cplusplus
}
#endif
