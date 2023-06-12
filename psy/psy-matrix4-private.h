
#include <glm/glm.hpp>

#include "psy-matrix4.h"

glm::mat4&
psy_matrix4_get_priv_reference(PsyMatrix4 *self);

glm::mat4 *
psy_matrix4_get_priv_pointer(PsyMatrix4 *self);
