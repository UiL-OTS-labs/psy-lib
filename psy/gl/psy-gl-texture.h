
#ifndef PSY_GL_TEXTURE_H
#define PSY_GL_TEXTURE_H

#include "psy-texture.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_TEXTURE psy_gl_texture_get_type()
G_DECLARE_FINAL_TYPE(PsyGlTexture, psy_gl_texture, PSY, GL_TEXTURE, PsyTexture)

/*
typedef struct _PsyGlTextureClass {
    PsyTextureClass parent_class;
} PsyGlTextureClass;
*/

G_MODULE_EXPORT PsyGlTexture *
psy_gl_texture_new(void);

G_MODULE_EXPORT PsyGlTexture *
psy_gl_texture_new_for_file(GFile *file);

G_MODULE_EXPORT PsyGlTexture *
psy_gl_texture_new_for_path(const gchar *path);

G_MODULE_EXPORT guint
psy_gl_texture_get_object_id(PsyGlTexture *self);

G_END_DECLS

#endif
