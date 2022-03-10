#ifndef PSY_GL_PROGRAM_H
#define PSY_GL_PROGRAM_H

#include "psy-program.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_PROGRAM psy_gl_program_get_type()
G_DECLARE_FINAL_TYPE(PsyGlProgram, psy_gl_program, PSY, GL_PROGRAM, PsyProgram)


G_MODULE_EXPORT PsyGlProgram*
psy_gl_program_new();

G_MODULE_EXPORT guint 
psy_gl_program_get_object_id(PsyGlProgram* program);


G_END_DECLS

#endif
