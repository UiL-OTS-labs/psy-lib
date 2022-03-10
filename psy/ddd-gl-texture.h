
#ifndef DDD_GL_TEXTURE_H
#define DDD_GL_TEXTURE_H

#include "ddd-texture.h"

G_BEGIN_DECLS

#define DDD_TYPE_GL_TEXTURE ddd_gl_texture_get_type()
G_DECLARE_FINAL_TYPE(DddGlTexture, ddd_gl_texture, DDD, GL_TEXTURE, DddTexture)

/*
typedef struct _DddGlTextureClass {
    DddTextureClass parent_class;
} DddGlTextureClass;
*/

G_MODULE_EXPORT DddGlTexture*
ddd_gl_texture_new();

G_MODULE_EXPORT DddGlTexture*
ddd_gl_texture_new_for_file(GFile* file);

G_MODULE_EXPORT DddGlTexture*
ddd_gl_texture_new_for_path(const gchar* path);

G_END_DECLS

#endif
