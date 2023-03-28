#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include "System.hpp"
#include "InputMap.hpp"
#include "Signatures.hpp"
#include "glm/glm.hpp"

class RenderSystem : public System
{
    public:
    RenderSystem(MessageBus& message_bus, InputMap& input_map);
    ~RenderSystem();

    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);
    void Update(float delta_time);

    private:

    uint32_t m_shader_program;
    uint32_t m_m_location;
    uint32_t m_v_location;
    uint32_t m_p_location;
    
    glm::vec3 m_eye = { 0, 0, 0 };
    glm::vec3 m_look = { 0, 0, -1 };
    glm::vec3 m_up = { 0, 1, 0 };
};

#endif // RENDER_SYSTEM_HPP