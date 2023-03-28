#ifndef RIGID_BODY_HPP
#define RIGID_BODY_HPP

#include "glm/glm.hpp"

struct RigidBody
{
    glm::vec3 acceleration;
    glm::vec3 velocity;
};

#endif // RIGID_BODY_HPP