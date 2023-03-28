#ifndef MESH_HPP
#define MESH_HPP

#include "glm/glm.hpp"

struct Vertex
{
   float position[3];
   float normal[3];
   float color[3];
   float tex_position[2];
};

enum TextureType
{
    DIFFUSE,
    SPECULAR,
};

struct Texture
{
    uint32_t id;
    TextureType type;
    uint32_t unit;
};

struct Mesh
{
    uint32_t num_vertices;
    uint32_t num_indices;
    uint32_t num_textures;
    Vertex* vertices;
    uint16_t* indices;
    Texture* textures;
};

#endif // MESH_HPP