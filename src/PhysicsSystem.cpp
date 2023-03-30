#include <stdio.h>
#include <limits>
#include <algorithm>

#include "PhysicsSystem.hpp"

struct Box
{
    float x, y;
    float w, h;
    float vx, vy;
};

float SweptAABB(glm::vec2 velocity, glm::vec2 min, glm::vec2 max, glm::vec2 other_min, glm::vec2 other_max, float& normal_x, float& normal_y);
void GetInvEntryExit(float v, float min1, float max1, float min2, float max2, float& inv_entry, float& inv_exit);
void GetEntryExit(float v, float inv_entry, float inv_exit, float& entry, float& exit);

PhysicsSystem::PhysicsSystem(MessageBus& message_bus) : 
    System(message_bus, PHYSICS_SYSTEM_SIGNATURE)
{

}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::HandleMessage(Message message)
{
    
}

void PhysicsSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    BoundingBox& bounding_box = m_component_manager->GetComponent<BoundingBox>(entity_id);

    uint16_t collision_id;
    float move_time = delta_time;
    float normal_x = 0;
    float normal_y = 0;

    rigid_body.velocity[0] += rigid_body.acceleration[0] * move_time;
    rigid_body.velocity[1] += (rigid_body.acceleration[1] - 9.8 * 2.5) * move_time;

    glm::vec2 half_extent = glm::vec2(bounding_box.extent[0] / 2, bounding_box.extent[1] / 2);
    glm::vec2 min = glm::vec2(transform.position[0] - half_extent[0], transform.position[1] - half_extent[1]);
    glm::vec2 max = glm::vec2(transform.position[0] + half_extent[0], transform.position[1] + half_extent[1]);

    glm::vec2 bp_min = glm::vec2(rigid_body.velocity[0] > 0 ? min[0] : (transform.position[0] + rigid_body.velocity[0] * delta_time) - half_extent[0],
                                    rigid_body.velocity[1] > 0 ? min[1] : (transform.position[1] + rigid_body.velocity[1] * delta_time) - half_extent[1]);
    glm::vec2 bp_max = glm::vec2(rigid_body.velocity[0] > 0 ? (transform.position[0] + rigid_body.velocity[0] * delta_time) + half_extent[0] : max[0],
                                    rigid_body.velocity[1] > 0 ? (transform.position[1] + rigid_body.velocity[1] * delta_time) + half_extent[1] : max[1]);

    uint32_t num_entities = m_entity_manager->GetNumEntities();

    for(uint32_t other_entity_id = 0; other_entity_id < num_entities; other_entity_id++)
    {
        if(other_entity_id != entity_id &&
            m_entity_manager->GetEntityState(other_entity_id) == EntityState::ACTIVE &&
            m_entity_manager->GetEntitySignature(other_entity_id) & COLLISION_SYSTEM_SIGNATURE)
        {
            Transform& other_transform = m_component_manager->GetComponent<Transform>(other_entity_id);
            BoundingBox& other_bounding_box = m_component_manager->GetComponent<BoundingBox>(other_entity_id);

            glm::vec2 other_half_extent = glm::vec2(other_bounding_box.extent[0] / 2, other_bounding_box.extent[1] / 2);
            glm::vec2 other_min = glm::vec2(other_transform.position[0] - other_half_extent[0], other_transform.position[1] - other_half_extent[1]);
            glm::vec2 other_max = glm::vec2(other_transform.position[0] + other_half_extent[0], other_transform.position[1] + other_half_extent[1]);

            if(bp_min[0] < other_max[0] && bp_max[0] >= other_min[0] && bp_min[1] < other_max[1] && bp_max[1] >= other_min[1])
            {
                float collision_time = SweptAABB(glm::vec2(rigid_body.velocity[0] * delta_time, rigid_body.velocity[1] * delta_time), 
                                                    min, max, other_min, other_max, normal_x, normal_y);
                if(collision_time * delta_time < move_time)
                {
                    move_time = collision_time * delta_time;
                    collision_id = other_entity_id;
                }
            }

        }
    }

    if(normal_x != 0)
    {
        rigid_body.velocity[0] = 0;
    }

    if(normal_y != 0)
    {
        rigid_body.velocity[1] = 0;
    }

    transform.position[0] += rigid_body.velocity[0] * move_time;
    transform.position[1] += rigid_body.velocity[1] * move_time;

    float remaining_time = delta_time - move_time;

    float dot_prod = (rigid_body.velocity[0] * normal_y + rigid_body.velocity[1] * normal_x) * remaining_time;
    glm::vec2 new_vel = glm::vec2(dot_prod * normal_y, dot_prod * normal_x);

    transform.position[0] += new_vel[0];
    transform.position[1] += new_vel[1];

    if(move_time < delta_time && normal_y == 1)
    {
        Message message;
        message.message_type = MessageType::COLLISION;
        message.message_data = (entity_id << 16) + collision_id;
        m_message_bus.PostMessage(message);
    }

}

float SweptAABB(glm::vec2 velocity, glm::vec2 min, glm::vec2 max, glm::vec2 other_min, glm::vec2 other_max, float& normal_x, float& normal_y)
{
    float x_inv_entry, y_inv_entry;
    float x_inv_exit, y_inv_exit;

    GetInvEntryExit(velocity[0], min[0], max[0], other_min[0], other_max[0], x_inv_entry, x_inv_exit);
    GetInvEntryExit(velocity[1], min[1], max[1], other_min[1], other_max[1], y_inv_entry, y_inv_exit);

    float x_entry, y_entry; 
    float x_exit, y_exit;

    GetEntryExit(velocity[0], x_inv_entry, x_inv_exit, x_entry, x_exit);
    GetEntryExit(velocity[1], y_inv_entry, y_inv_exit, y_entry, y_exit);
    
    float entry_time = std::max(x_entry, y_entry);
    float exit_time = std::min(x_exit, y_exit);

    if (entry_time > exit_time || x_entry < 0.0f && y_entry < 0.0f || x_entry > 1.0f || y_entry > 1.0f) 
    { 
        normal_x = 0.0f; 
        normal_y = 0.0f; 
        return 1.0f; 
    }
    else // if there was a collision 
    { 
        // calculate normal of collided surface
        if (x_entry > y_entry) 
        { 
            if (x_inv_entry < 0.0f) 
            { 
                normal_x = 1.0f; 
                normal_y = 0.0f; 
            } 
            else 
            { 
                normal_x = -1.0f; 
                normal_y = 0.0f;
            } 
        } 
        else 
        { 
            if (y_inv_entry < 0.0f) 
            { 
                normal_x = 0.0f; 
                normal_y = 1.0f; 
            } 
            else 
            { 
                normal_x = 0.0f; 
                normal_y = -1.0f; 
            } 
        }
        return entry_time;
    }
}

void GetInvEntryExit(float v, float min1, float max1, float min2, float max2, float& inv_entry, float& inv_exit)
{
    if(v > 0)
    {
        inv_entry = min2 - max1;
        inv_exit = max2 - min1;
    }
    else
    {
        inv_entry = max2 - min1;
        inv_exit = min2 - max1;
    }
}

void GetEntryExit(float v, float inv_entry, float inv_exit, float& entry, float& exit)
{
    if(v == 0)
    {
        entry = -std::numeric_limits<float>::infinity();
        exit = std::numeric_limits<float>::infinity();
    }
    else
    {
        entry = inv_entry / v;
        exit = inv_exit / v;
    }
}