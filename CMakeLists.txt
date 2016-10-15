cmake_minimum_required(VERSION 2.8.11)

project(tanks)

add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
)

add_subdirectory(sound)

add_executable(tanks WIN32
    cm_sound.h
    cm_variable.cpp
    cm_variable.h
    g_button.cpp
    g_main.cpp
    g_main.h
    g_menu.cpp
    g_menu.h
    g_network.cpp
    g_object.cpp
    g_particles.cpp
    g_tank.cpp
    g_world.cpp
    g_world.h
    gl_main.cpp
    gl_main.h
    keys.h
    local.h
    models.h
    net_chan.cpp
    net_main.cpp
    net_main.h
    net_msg.cpp
    oed_error.h
    oed_files.h
    oed_mem.h
    oed_shared.h
    oed_tools.h
    oed_types.h
    r_draw.cpp
    r_font.cpp
    r_image.cpp
    r_main.cpp
    r_main.h
    r_model.cpp
    r_model.h
    r_particle.h
    resource.rc
    resource.h
    shared.h
    win_main.cpp
    win_main.h
)

target_link_libraries(tanks
    # project libraries
    sound
    # system libraries
    ws2_32
    opengl32
    Pathcch
)

set_target_properties(tanks PROPERTIES OUTPUT_NAME "Tanks!")
