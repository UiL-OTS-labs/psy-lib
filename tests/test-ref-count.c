
#include "psy-visual-stimulus.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <assert.h>

#include <glib.h>
#include <psy-color.h>
#include <psy-circle.h>
#include <backend_gtk/psy-gtk-window.h>

gboolean verbose;

GOptionEntry options[] = {
    {"verbose", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &verbose, "Run the suite verbosely",""},
    {0}
};


PsyWindow* window = NULL;

static int
init_window(void) {
    window = PSY_WINDOW(psy_gtk_window_new());
    if (window)
        return 0;
    else
        return 1;
}

static int 
destoy_window(void) {
    assert(G_OBJECT(window)->ref_count == 1);
    g_object_unref(window);
    window = NULL;
    return 0;
}

static void
ref_starts_with_one(void)
{
    PsyCircle* circle = g_object_new(PSY_TYPE_CIRCLE, "window", window, NULL);
    PsyColor* color = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject* circle_gobj = G_OBJECT(circle);
    GObject* color_gobj = G_OBJECT(color);

    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(circle);
    g_object_unref(color);
}

static void
ref_set_method(void)
{
    PsyCircle* circle = g_object_new(PSY_TYPE_CIRCLE, "window", window, NULL);
    PsyColor* color = g_object_new(PSY_TYPE_COLOR, NULL);

    // Cast to conveniently obtain the reference count
    GObject* circle_gobj = G_OBJECT(circle);
    GObject* color_gobj = G_OBJECT(color);

    /*
     * This is (transfer none), so the circle does not own the reference,
     * but has a reference to it.
     * Both we and the Circle should free it.
     */ 
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), color);
    
    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 2);

    // The circle disposes its reference to color
    g_object_unref(circle);
    
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(color);
}

static void
ref_set_property(void)
{
    PsyCircle* circle = g_object_new(PSY_TYPE_CIRCLE, "window", window, NULL);
    PsyColor* color = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject* circle_gobj = G_OBJECT(circle);
    GObject* color_gobj = G_OBJECT(color);
    
    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

     // Circle is owning a reference
    g_object_set(
            circle,
            "color", color,
            NULL
            );
    
    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 2);

    g_object_unref(circle); // Circle disposes its reference on color
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(color);
}


int main(int argc, char** argv) {

    GOptionContext* context = g_option_context_new("");
    g_option_context_add_main_entries(context, options, NULL);
    GError* error = NULL;

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }

    CU_initialize_registry();

    if (verbose)
        CU_basic_set_mode(CU_BRM_VERBOSE);

    CU_Suite* suite = CU_add_suite("test reference count", init_window, destoy_window);
    CU_add_test(suite, "Object start with reference of 1", ref_starts_with_one);
    CU_add_test(suite, "Objects ref set method", ref_set_method);
    CU_add_test(suite, "Objects ref set property", ref_set_property);

    CU_basic_run_tests();

    CU_cleanup_registry();
    g_option_context_free(context);
}
