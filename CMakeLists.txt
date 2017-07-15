cmake_minimum_required(VERSION 2.8.11)

project(tanks)

add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
)

add_subdirectory(shared)
add_subdirectory(sound)
add_subdirectory(physics)

set(tanks-sources
    g_button.cpp
    g_client.cpp
    g_main.cpp
    g_main.h
    g_menu.cpp
    g_menu.h
    g_network.cpp
    g_object.cpp
    g_particles.cpp
    g_server.cpp
    g_tank.cpp
    g_usercmd.cpp
    g_usercmd.h
    g_world.cpp
    g_world.h
    keys.h
    net_chan.cpp
    net_main.cpp
    net_main.h
    net_msg.cpp
    r_draw.cpp
    r_font.cpp
    r_image.cpp
    r_main.cpp
    r_main.h
    r_model.cpp
    r_model.h
    r_particle.h
    r_window.cpp
    r_window.h
    resource.rc
    resource.h
    win_main.cpp
    win_main.h
)

add_executable(tanks WIN32
    ${tanks-sources}

    precompiled.cpp
    precompiled.h
)

target_link_libraries(tanks
    # project libraries
    shared
    sound
    physics
    # system libraries
    ws2_32
    opengl32
    Pathcch
)

# Set up precompiled header
set_source_files_properties(precompiled.cpp PROPERTIES COMPILE_FLAGS /Ycprecompiled.h)
set_source_files_properties(${tanks-sources} PROPERTIES COMPILE_FLAGS /Yuprecompiled.h)

set_target_properties(tanks PROPERTIES OUTPUT_NAME "Tanks!")
