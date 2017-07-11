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

class vObject
{
public:
    void * operator new (size_t s) { return mem::alloc( s ); }
    void operator delete (void *ptr) { mem::free( ptr ); }
};

class vMain
{
public:
    virtual int message (char const* message, ...) = 0;
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

static cVec2 rot (cVec2 v, float rad)
{
    return cVec2( v.x*cos(rad) - v.y*sin(rad),
                  v.y*cos(rad) + v.x*sin(rad) );
}
