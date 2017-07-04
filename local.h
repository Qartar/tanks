/*
===============================================================================

Name    :   local.h

Purpose :   General top-down include file

===============================================================================
*/

#pragma once

#include "shared.h"

#include <windows.h>
//  data type headers
#include "r_particle.h"
#include "r_model.h"
//  common headers
#include "cm_sound.h"
#include "cm_variable.h"
//  network headers
#include "net_main.h"
//  game headers
#include "g_world.h"
#include "g_menu.h"
#include "g_main.h"
//  rendering headers
#include "r_main.h"
//  opengl headers
#include "gl_main.h"
//  windows headers
#include "win_main.h"

#define TANK_MOVE       0
#define TANK_IDLE       1
#define TANK_FIRE       2
#define TANK_EXPLODE    3
#define BULLET_EXPLODE  4
#define TURRET_MOVE     5
#define NUM_SOUNDS      6
