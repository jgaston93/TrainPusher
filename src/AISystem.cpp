#include "AISystem.hpp"

#include <cstdio>


AISystem::AISystem(MessageBus& message_bus) : 
    System(message_bus, AI_SYSTEM_SIGNATURE)
{
}

AISystem::~AISystem()
{
}

void AISystem::HandleMessage(Message message)
{
    
}

void AISystem::HandleEntity(uint32_t entity_id, float delta_time)
{

}