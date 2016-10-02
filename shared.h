/*
===============================================================================

Name    :   shared.h

Purpose :   Global Definitions ; included first by all header files

Date    :   10/16/2004

===============================================================================
*/

#pragma once

#define WIN32_LEAN_AND_MEAN     // exclude rarely used Windows crap

#define APP_CLASSNAME       "Tanks!"

#define PORT_SERVER     28101
#define PORT_CLIENT     28110

#define SHORT_SWAP(a)   (((a&0xffff)>>8)|((a&0xffff)<<8))&0xffff
#define LONG_SWAP(a)    ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24)

#define BIG_SHORT(a)    SHORT_SWAP(a)

#define LITTLE_LONG(a)  (a)

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define OED_LIB

#include "oed_shared.h"
#include "oed_types.h"
#include "oed_error.h"
#include "oed_files.h"
#include "oed_tools.h"

#define strnicmp    _strnicmp
#define stricmp     _stricmp

#define DEG2RAD(a)  deg2rad(a)
#define RAD2DEG(a)  rad2deg(a)

class vObject
{
public:
    void * operator new (size_t s) { return mem::alloc( s ); }
    void operator delete (void *ptr) { mem::free( ptr ); }
};

class vMain
{
public:
    virtual int Message (char *szMessage, ...) = 0;
};

extern vMain    *pMain;

//
//  default static create/destroy routines
//

//  the problem with this is that VS intellisense doesn't
//  recognize p##x when programming other modules

#define DEF_CREATE_DESTROY(x)           \
    v##x    *p##x = NULL;               \
void v##x::Create () {                  \
    p##x = (v##x *) new c##x; }         \
                                        \
void v##x::Destroy () {                 \
    ((c##x *)p##x)->~c##x(); delete p##x; p##x = NULL; }        

static cVec2 rot (cVec2 v, float deg)
{
    return cVec2( v.x*cos(deg2rad(deg)) - v.y*sin(deg2rad(deg)),
        v.y*cos(deg2rad(deg)) + v.x*sin(deg2rad(deg)) );
}

#define UPGRADE_FRAC    g_upgrade_frac->getFloat()
#define UPGRADE_PENALTY g_upgrade_penalty->getFloat()
#define UPGRADE_MIN     g_upgrade_min->getFloat()

typedef struct game_client_s
{
    vec3    color;

    float   damage_mod;
    float   armor_mod;
    float   refire_mod;
    float   speed_mod;

    int     upgrades;
} game_client_t;
