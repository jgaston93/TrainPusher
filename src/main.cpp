#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>
#include <fstream>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ComponentManager.hpp"
#include "EntityManager.hpp"

#include "PlayerInputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "UISystem.hpp"
#include "AISystem.hpp"
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


InputMap* input_map;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    bool is_pressed = input_map->IsPressed(key);
    if(action == GLFW_PRESS)
    {
        is_pressed = true;
    }
    else if(action == GLFW_RELEASE)
    {
        is_pressed = false;
    }
    input_map->SetIsPressed(key, is_pressed);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    bool is_pressed = input_map->IsPressed(button);
    if(action == GLFW_PRESS)
    {
        is_pressed = true;
    }
    else if(action == GLFW_RELEASE)
    {
        is_pressed = false;
    }
    input_map->SetIsPressed(button, is_pressed);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    input_map->SetMousePosX(xpos);
    input_map->SetMousePosY(ypos);
}

void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager);

int main(int argv, char* args[])
{ 
    GLFWwindow* window;
    GLuint vertex_buffer, element_buffer;
 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
 
    window = glfwCreateWindow(640, 480, "Train Pusher", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCursorPos(window, 0, 0);
 
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    uint32_t num_inputs = 13;
    uint32_t input_list[] = { GLFW_KEY_LEFT,
                                GLFW_KEY_RIGHT,
                                GLFW_KEY_UP,
                                GLFW_KEY_DOWN,
                                GLFW_KEY_W,
                                GLFW_KEY_S,
                                GLFW_KEY_A,
                                GLFW_KEY_D,
                                GLFW_KEY_E,
                                GLFW_KEY_Q,
                                GLFW_KEY_SPACE,
                                GLFW_MOUSE_BUTTON_RIGHT,
                                GLFW_MOUSE_BUTTON_LEFT };

    input_map = new InputMap(num_inputs);
    for(uint32_t i = 0; i < num_inputs; i++)
    {
        input_map->AddInput(input_list[i]);
    }
    
    const uint32_t num_entities = 3;
    EntityManager entity_manager(num_entities);
    ComponentManager component_manager(num_entities);

    const uint32_t num_messages = 1024;
    const uint32_t num_systems = 5;
    MessageBus message_bus(num_messages, num_systems);

    PlayerInputSystem player_input_system(message_bus, *input_map);
    player_input_system.SetEntityManager(&entity_manager);
    player_input_system.SetComponentManager(&component_manager);
    PhysicsSystem physics_system(message_bus);
    physics_system.SetEntityManager(&entity_manager);
    physics_system.SetComponentManager(&component_manager);
    RenderSystem render_system(message_bus, *input_map);
    render_system.SetEntityManager(&entity_manager);
    render_system.SetComponentManager(&component_manager);
    UISystem ui_system(message_bus, window);
    ui_system.SetEntityManager(&entity_manager);
    ui_system.SetComponentManager(&component_manager);

    GenerateEntities(entity_manager, component_manager);
    
    std::chrono::time_point<std::chrono::steady_clock> prev_time = std::chrono::steady_clock::now();
    uint32_t num_frames = 0;
    uint32_t MS_PER_FRAME = 16;

    bool is_running = true;

    while (!glfwWindowShouldClose(window))
    {
        std::chrono::time_point<std::chrono::steady_clock> current_time = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - prev_time).count() * 1e-3;
        prev_time = current_time;

        glfwPollEvents();
        
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        message_bus.Update();
        player_input_system.Update(delta_time);
        physics_system.Update(delta_time);
        glEnable(GL_DEPTH_TEST);
        render_system.Update(delta_time);
        glDisable(GL_DEPTH_TEST);
        ui_system.Update(delta_time);

        glfwSwapBuffers(window);
        
        // Calculate time to sleep and sleep if necessary
        std::chrono::time_point<std::chrono::steady_clock> next_frame_time = current_time + std::chrono::milliseconds(MS_PER_FRAME);
        std::this_thread::sleep_until(next_frame_time);
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
    
    return 0;
}


void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager)
{
    uint32_t entity_id = 0;

    // Camera Entity
    Transform transform;
    transform.position = glm::vec3(0);
    transform.rotation = glm::vec3(0);
    transform.scale = glm::vec3(0);

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntityTag(entity_id, "camera");

    component_manager.AddComponent<Transform>(entity_id, transform);

    entity_id++;

    transform.position = glm::vec3(0, 2, -10);

    Quad quad;
    quad.extent = glm::vec2(1, 1);
    quad.color = glm::vec3(1, 0, 0);

    BoundingBox bounding_box;
    bounding_box.extent = glm::vec2(1, 1);

    PlayerInput player_input;
    player_input.can_jump = false;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, PLAYER_INPUT_SYSTEM_SIGNATURE | RENDER_SYSTEM_SIGNATURE | PHYSICS_SYSTEM_SIGNATURE | COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "player");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);
    component_manager.AddComponent<PlayerInput>(entity_id, player_input);

    entity_id++;

    transform.position = glm::vec3(0, -2.5, -10);

    quad.extent = glm::vec2(10, 5);
    quad.color = glm::vec3(0, 1, 0);

    bounding_box.extent = glm::vec2(10, 5);

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;

    printf("Num Entities: %d\n", entity_id);
}
