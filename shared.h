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

#if 1

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

#else

#define MAX_STRING      1024
#define LONG_STRING     256
#define SHORT_STRING    32

//#define DEBUG_MEM

#ifdef DEBUG_MEM
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // DEBUG_MEM

#pragma warning (disable:4244)  //  double to float
#pragma warning (disable:4267)  //  size_t to int

#define ERROR_NONE      0
#define ERROR_FAIL      1
#define ERROR_UNKNOWN   -1

#ifndef NULL
#define NULL 0
#endif // NULL

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define DEG2RAD(a) ((a)*M_PI)/180.0F

#define frand() (((float)rand())/32767.0f)
#define crand() ((frand()-0.5f)*2)

typedef unsigned char   byte;

typedef class cVec2
{
public:
    float   x, y;

    cVec2 () {}
    cVec2 (float X, float Y) : x(X), y(Y) {}

    float operator[] (int i) { return ( *((float*)(this)+i) ); }
    cVec2 rot (float deg) {
        return cVec2(
            ( x*cos(DEG2RAD(deg)) - y*sin(DEG2RAD(deg)) ),
            ( y*cos(DEG2RAD(deg)) + x*sin(DEG2RAD(deg)) ) ); }

    cVec2 operator+ (cVec2 &A) { return cVec2(x + A.x,y + A.y); }
    cVec2 operator- (cVec2 &A) { return cVec2(x - A.x,y - A.y); }
    cVec2 operator* (float fl) { return cVec2(x * fl, y * fl); }

} vec2;

typedef class cVec4
{
public:
    float   r, g, b, a;

    cVec4 () {}
    cVec4 (float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {}

    float operator[] (int i) { return ( *((float*)(this)+i) ); }
} vec4;

#define min(a,b) ( a < b ? a : b )
#define max(a,b) ( a > b ? a : b )
#define clamp(a,b,c) ( a < b ? b : ( a > c ? c : a ) )

static char *va(char *format, ...)
{
    va_list     argptr;
    static char     string[MAX_STRING];
    
    va_start (argptr, format);
    vsprintf (string, format,argptr);
    va_end (argptr);

    return string;  
}

static void fmt (char *szDest, char *szMessage, ...)
{
    va_list     apList;

    va_start( apList, szMessage );
    vsprintf( szDest, szMessage, apList );
    va_end( apList );
}

#endif

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
