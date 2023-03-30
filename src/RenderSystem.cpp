#include <vector>
#include <algorithm>
#include <cstdio>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "RenderSystem.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct VertexData
{
    float position[3];
    float color[3];
    float tex_coord[2];
};

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 M;\n"
"uniform mat4 V;\n"
"uniform mat4 P;\n"
"layout(location = 0)in vec3 vPos;\n"
"layout(location = 1)in vec3 vCol;\n"
"layout(location = 2)in vec2 vTexCoord;\n"
"out vec2 fTexCoord;\n"
"out vec3 fCol;\n"
"out vec3 fFragPos;\n"
"void main()\n"
"{\n"
"    gl_Position = P * V * M * vec4(vPos, 1.0);\n"
"    fCol = vCol;\n"
"    fTexCoord = vTexCoord;\n"
"    fFragPos = vec3(M * vec4(vPos, 1.0));\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec3 fCol;\n"
"in vec2 fTexCoord;\n"
"in vec3 fFragPos;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"    float ambient_strength = 0.5;\n"
"    vec3 ambient = ambient_strength * vec3(1.0, 1.0, 1.0);\n"
"    fragColor = vec4(ambient  * fCol, 1.0);\n"
"}\n";

RenderSystem::RenderSystem(MessageBus& message_bus, InputMap& input_map) : 
    System(message_bus, RENDER_SYSTEM_SIGNATURE)
{
    // uint32_t texture_index = 0;
    // glGenTextures(1, &m_textures[texture_index]);
    // glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // int32_t num_channels;
    // stbi_set_flip_vertically_on_load(true);
    // unsigned char *data = stbi_load("assets/BrickTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // stbi_image_free(data);
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    GLint isCompiled = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(vertex_shader); // Don't leak the shader.
        return;
    }
 
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    isCompiled = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(fragment_shader); // Don't leak the shader.
        return;
    }
 
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, vertex_shader);
    glAttachShader(m_shader_program, fragment_shader);
    glLinkProgram(m_shader_program);
    
    GLint status; 
    glGetProgramiv( m_shader_program, GL_LINK_STATUS, &status ); 
    if( GL_FALSE == status ) {
        GLint maxLength = 0;
        glGetProgramiv(m_shader_program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(m_shader_program, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteProgram(m_shader_program); // Don't leak the shader.
        return;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    m_m_location = glGetUniformLocation(m_shader_program, "M");
    m_v_location = glGetUniformLocation(m_shader_program, "V");
    m_p_location = glGetUniformLocation(m_shader_program, "P");
}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::HandleMessage(Message message)
{
    
}

void RenderSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Quad& quad = m_component_manager->GetComponent<Quad>(entity_id);

    float half_width = quad.extent[0] / 2;
    float half_height = quad.extent[1] / 2;

    VertexData vertex_data[] = { { -half_width,  half_height, 0, quad.color[0], quad.color[1], quad.color[2], 0, 0 },
                                 { -half_width, -half_height, 0, quad.color[0], quad.color[1], quad.color[2], 0, 0 },
                                 {  half_width, -half_height, 0, quad.color[0], quad.color[1], quad.color[2], 0, 0 },
                                 {  half_width,  half_height, 0, quad.color[0], quad.color[1], quad.color[2], 0, 0 } };

    uint16_t index_data[] = { 0, 1, 2, 2, 3, 0 };
    
    // Model Matrix
    glm::mat4 model_matrix = glm::mat4(1.0);

    // Rotation
    model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation[2]), glm::vec3(0.0, 0.0, 1.0));
    model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation[1]), glm::vec3(0.0, 1.0, 0.0));
    model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation[0]), glm::vec3(1.0, 0.0, 0.0));

    // Translation
    model_matrix = glm::translate(model_matrix, transform.position);
                
    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(m_eye, m_look, m_up);
    
    // Perspective Matrix
    glm::mat4 perspective_matrix = glm::perspective(glm::pi<float>() * 0.25, 4.0 / 3.0, 0.1, 100.0);

    glUseProgram(m_shader_program);
    glUniformMatrix4fv(m_m_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(m_v_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(m_p_location, 1, GL_FALSE, glm::value_ptr(perspective_matrix));

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}


void RenderSystem::Update(float delta_time)
{
    uint32_t camera_entity_id = m_entity_manager->GetEntityId("camera");
    Transform& camera_transform = m_component_manager->GetComponent<Transform>(camera_entity_id);

    uint32_t player_entity_id = m_entity_manager->GetEntityId("player");
    Transform& player_transform = m_component_manager->GetComponent<Transform>(player_entity_id);

    float camera_speed = 0.05;
    glm::vec3 offset = glm::vec3(0, 2, 10);

    glm::vec3 desired_position = glm::vec3(0);

    camera_transform.position[0] = camera_transform.position[0] + (player_transform.position[0] - camera_transform.position[0]) * camera_speed;
    camera_transform.position[1] = camera_transform.position[1] + (player_transform.position[1] - camera_transform.position[1]) * camera_speed;

    m_eye = camera_transform.position + offset;

    // Rotation
    glm::mat4 rotation_matrix = glm::mat4(1.0);
    rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camera_transform.rotation[0]), glm::vec3(1.0, 0.0, 0.0));
    rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camera_transform.rotation[1]), glm::vec3(0.0, 1.0, 0.0));
    rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camera_transform.rotation[2]), glm::vec3(0.0, 0.0, 1.0));

    glm::vec4 forward = { 0, 0, -1 , 0};
    forward = rotation_matrix * forward;

    m_look[0] = m_eye[0] + forward[0];
    m_look[1] = m_eye[1] + forward[1];
    m_look[2] = m_eye[2] + forward[2];
 
    int32_t vpos_location = glGetAttribLocation(m_shader_program, "vPos");
    int32_t vcol_location = glGetAttribLocation(m_shader_program, "vCol");
    int32_t vtexcoord_location = glGetAttribLocation(m_shader_program, "vTexCoord");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(vtexcoord_location);
    glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) (sizeof(float) * 6));

    uint32_t num_entities = m_entity_manager->GetNumEntities();
    
    for(uint32_t i = 0; i < num_entities; i++)
    {
        if(m_entity_manager->GetEntityState(i) == EntityState::ACTIVE &&
            (m_entity_manager->GetEntitySignature(i) & m_system_signature))
        {
            HandleEntity(i, delta_time);
        }
    }
}