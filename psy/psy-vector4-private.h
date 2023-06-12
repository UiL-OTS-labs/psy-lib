
#pragma once

#include <glm/glm.hpp>

#include "psy-vector4.h"

glm::vec4&
psy_vector4_get_priv_reference(PsyVector4 *self);

glm::vec4 *
psy_vector4_get_priv_pointer(PsyVector4 *self);
