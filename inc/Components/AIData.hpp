#ifndef AI_DATA_HPP
#define AI_DATA_HPP

#include "glm/glm.hpp"

struct AIData
{
    float speed;
    bool alive;
    float initial_height;
    glm::vec3 position;
    glm::vec3 rotation;
};

#endif // AI_DATA_HPP