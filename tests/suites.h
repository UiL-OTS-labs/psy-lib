
#pragma once

#include <glib.h>

int
add_audio_channel_mapping_suite(void);

int
add_audio_suite(const gchar *backend);

int
add_audio_utils_suite(void);

int
add_canvas_suite(void);

int
add_color_suite(void);

int
add_image_suite(void);

int
add_gl_canvas_suite(void);

int
add_gl_utils_suite(void);

int
add_matrix4_suite(void);

int
add_parallel_suite(gint port_num);

int
add_picture_suite(void);

int
add_queue_suite(void);

int
add_ref_count_suite(void);

int
add_stepping_suite(void);

int
add_text_suite(void);

int
add_time_utilities_suite(void);

int
add_utility_suite(void);

int
add_visual_stimulus_suite(void);

int
add_visual_stimuli_suite(void);

int
add_vector_suite(void);

int
add_vector3_suite(void);

int
add_vector4_suite(void);

int
add_wave_suite(void);
