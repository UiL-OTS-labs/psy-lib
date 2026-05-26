
#pragma once

#include <psylib.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct UnitTestUtilsInit {
    const char    *log_file;
    GLogLevelFlags log_level;
    bool           save_pictures;
    const char   **domains; // null terminated; may be null
} UnitTestUtilsInit;

void
unit_test_utils_init(UnitTestUtilsInit *init_info);

gboolean
save_images(void);

void
save_image_tmp_png(PsyImage *image, const char *name_fmt, ...);

G_END_DECLS
