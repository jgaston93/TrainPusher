#include <cstdio>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "PlayerInputSystem.hpp"


PlayerInputSystem::PlayerInputSystem(MessageBus& message_bus, InputMap& input_map) : 
    System(message_bus, PLAYER_INPUT_SYSTEM_SIGNATURE), 
    m_input_map(input_map),
    m_can_jump(false)
{

}

PlayerInputSystem::~PlayerInputSystem()
{
}

void PlayerInputSystem::HandleMessage(Message message)
{
    if(message.message_type == MessageType::COLLISION)
    {
        m_can_jump = true;
    }
}

void PlayerInputSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);

    float max_player_speed = 10;
    float player_speed = 10;
    float acceleration = 0;
    float jump_force = 10;
    
    if(m_input_map.IsPressed(GLFW_KEY_LEFT))
    {
        acceleration -= player_speed;
    }
    if(m_input_map.IsPressed(GLFW_KEY_RIGHT))
    {
        acceleration += player_speed;
    }

    if(m_input_map.IsPressed(GLFW_KEY_SPACE) && m_can_jump)
    {
        rigid_body.velocity[1] = jump_force;
        m_can_jump = false;
    }

    rigid_body.acceleration[0] = acceleration;

    if(rigid_body.velocity[0] < -max_player_speed)
    {
        rigid_body.velocity[0] = -max_player_speed;
    }
    else if(rigid_body.velocity[0] > max_player_speed)
    {
        rigid_body.velocity[0] = max_player_speed;
    }

    if(rigid_body.velocity[1] < -max_player_speed)
    {
        rigid_body.velocity[1] = -max_player_speed;
    }
    else if(rigid_body.velocity[1] > max_player_speed)
    {
        rigid_body.velocity[1] = max_player_speed;
    }

    if(rigid_body.acceleration[0] == 0 && m_can_jump)
    {
        rigid_body.velocity[0] *= 0.9;
    }
}