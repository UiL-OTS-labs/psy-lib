

#include "psy-gl-context.h"
#include "psy-gl-fragment-shader.h"
#include "psy-gl-program.h"
#include "psy-gl-vbuffer.h"
#include "psy-gl-vertex-shader.h"
#include "psy-program.h"
#include "psy-vbuffer.h"

struct _PsyGlContext {
    PsyDrawingContext parent;
    GHashTable       *shader_programs;
};

G_DEFINE_FINAL_TYPE(PsyGlContext, psy_gl_context, PSY_TYPE_DRAWING_CONTEXT)

static void
psy_gl_context_init(PsyGlContext *self)
{
    self->shader_programs
        = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
}

static void
psy_gl_context_dispose(GObject *object)
{
    PsyGlContext *self = PSY_GL_CONTEXT(object);

    if (self->shader_programs) {
        g_hash_table_destroy(self->shader_programs);
        self->shader_programs = NULL;
    }

    G_OBJECT_CLASS(psy_gl_context_parent_class)->dispose(object);
}

static void
psy_gl_context_finalize(GObject *object)
{
    PsyGlContext *self = PSY_GL_CONTEXT(object);

    G_OBJECT_CLASS(psy_gl_context_parent_class)->finalize(object);
}

static PsyProgram *
psy_gl_create_program(PsyDrawingContext *self)
{
    g_assert(PSY_IS_GL_CONTEXT(self));
    return PSY_PROGRAM(psy_gl_program_new());
}

static PsyShader *
psy_gl_create_vertex_shader(PsyDrawingContext *self)
{
    g_assert(PSY_IS_GL_CONTEXT(self));
    return PSY_SHADER(psy_gl_vertex_shader_new());
}

static PsyShader *
psy_gl_create_fragment_shader(PsyDrawingContext *self)
{
    g_assert(PSY_IS_GL_CONTEXT(self));
    return PSY_SHADER(psy_gl_vertex_shader_new());
}

static PsyVBuffer *
psy_gl_create_vbuffer(PsyDrawingContext *self)
{
    g_assert(PSY_IS_GL_CONTEXT(self));
    return PSY_VBUFFER(psy_gl_vbuffer_new());
}

static void
psy_gl_context_class_init(PsyGlContextClass *class)
{
    GObjectClass           *gobject_class = G_OBJECT_CLASS(class);
    PsyDrawingContextClass *drawing_context_class
        = PSY_DRAWING_CONTEXT_CLASS(class);

    gobject_class->finalize = psy_gl_context_finalize;
    gobject_class->dispose  = psy_gl_context_dispose;

    drawing_context_class->create_program       = psy_gl_create_program;
    drawing_context_class->create_vertex_shader = psy_gl_create_vertex_shader;
    drawing_context_class->create_fragment_shader
        = psy_gl_create_fragment_shader;
    drawing_context_class->create_vbuffer = psy_gl_create_vbuffer;
}

/* ************ public functions ******************** */

PsyGlContext *
psy_gl_context_new(void)
{
    return g_object_new(PSY_TYPE_GL_CONTEXT, NULL);
}
