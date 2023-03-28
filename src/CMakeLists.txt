set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/assets/appicon.rc")

add_executable(trainPusher main.cpp
                             gl.c
                             MessageBus.cpp
                             System.cpp
                             ComponentManager.cpp
                             EntityManager.cpp
                             InputMap.cpp
                             PlayerInputSystem.cpp
                             PhysicsSystem.cpp
                             RenderSystem.cpp
                             UISystem.cpp
                             AISystem.cpp
                             ${APP_ICON_RESOURCE_WINDOWS})

target_include_directories(trainPusher PUBLIC ${CMAKE_SOURCE_DIR}/inc
                                                ${CMAKE_SOURCE_DIR}/inc/Components
                                                ${CMAKE_SOURCE_DIR}/inc/Systems
                                                ${CMAKE_SOURCE_DIR}/inc/Utils
                                                ${CMAKE_SOURCE_DIR}/inc/MessageBus)

target_link_options(trainPusher PUBLIC -static-libgcc -static-libstdc++ -static) # -mwindows 
target_link_libraries(trainPusher glfw OpenGL::GL)