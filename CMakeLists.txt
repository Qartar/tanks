cmake_minimum_required(VERSION 2.8.11)

project(tanks)

# Configure version file
find_package(git)

set(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake")
set(VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.h")

if(GIT_FOUND)
    add_custom_command(OUTPUT ${VERSION_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DPROJECT_NAME=${PROJECT_NAME}
            -DGIT_EXECUTABLE="${GIT_EXECUTABLE}"
            -DCONFIG_FILE="${SCRIPTS_DIR}/version.h.in"
            -DOUTPUT_FILE="${VERSION_FILE}"
            -P "${SCRIPTS_DIR}/version.cmake"
        DEPENDS
            ${SCRIPTS_DIR}/version.cmake
            ${SCRIPTS_DIR}/version.h.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    configure_file(${SCRIPTS_DIR}/version.h.in ${VERSION_FILE})
endif()

# Remove bad default compiler flags for Visual Studio
if(MSVC)
    string(REPLACE "/DWIN32 /D_WINDOWS /W3 /GR /EHsc" "/EHsc" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
)

add_compile_options(
    /W4             # Enable warning level 4
    /WX             # Enable warnings as errors
    /wd4706         # Disable C4706: assignment within conditional expression
    /permissive-    # Enable language conformance mode
    /std:c++17      # Enable C++17 language features
)

add_subdirectory(shared)
add_subdirectory(sound)
add_subdirectory(physics)
add_subdirectory(network)

set(TANKS_SOURCES
    game/g_button.cpp
    game/g_client.cpp
    game/g_menu.cpp
    game/g_menu.h
    game/g_network.cpp
    game/g_object.cpp
    game/g_object.h
    game/g_particles.cpp
    game/g_projectile.cpp
    game/g_projectile.h
    game/g_server.cpp
    game/g_session.cpp
    game/g_session.h
    game/g_tank.cpp
    game/g_tank.h
    game/g_usercmd.cpp
    game/g_usercmd.h
    game/g_world.cpp
    game/g_world.h
    render/r_draw.cpp
    render/r_font.cpp
    render/r_image.cpp
    render/r_main.cpp
    render/r_main.h
    render/r_model.cpp
    render/r_model.h
    render/r_particle.h
    render/r_window.cpp
    render/r_window.h
    system/resource.rc
    system/resource.h
    system/win_main.cpp
    system/win_main.h
    system/win_include.h
)

add_executable(tanks WIN32
    ${TANKS_SOURCES}
    ${VERSION_FILE}

    precompiled.cpp
    precompiled.h
)

target_link_libraries(tanks
    # project libraries
    shared
    sound
    physics
    network
    # system libraries
    Shcore
    ws2_32
    opengl32
    Pathcch
    Xinput
)

target_include_directories(tanks
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        game
        render
        system
)

# Set up precompiled header
set_source_files_properties(precompiled.cpp PROPERTIES COMPILE_FLAGS /Ycprecompiled.h OBJECT_OUTPUTS precompiled.pch)
set_source_files_properties(${TANKS_SOURCES} PROPERTIES COMPILE_FLAGS /Yuprecompiled.h OBJECT_DEPENDS precompiled.pch)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TANKS_SOURCES} precompiled.cpp precompiled.h)
source_group("\\" FILES precompiled.cpp precompiled.h)

set_target_properties(tanks PROPERTIES OUTPUT_NAME "Tanks!")
