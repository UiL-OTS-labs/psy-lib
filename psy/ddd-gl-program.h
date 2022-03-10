#ifndef DDD_GL_PROGRAM_H
#define DDD_GL_PROGRAM_H

#include "ddd-program.h"

G_BEGIN_DECLS

#define DDD_TYPE_GL_PROGRAM ddd_gl_program_get_type()
G_DECLARE_FINAL_TYPE(DddGlProgram, ddd_gl_program, DDD, GL_PROGRAM, DddProgram)


G_MODULE_EXPORT DddGlProgram*
ddd_gl_program_new();

G_MODULE_EXPORT guint 
ddd_gl_program_get_object_id(DddGlProgram* program);


G_END_DECLS

#endif
