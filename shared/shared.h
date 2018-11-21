/*
===============================================================================

Name    :   shared.h

Purpose :   Global Definitions ; included first by all header files

Date    :   10/16/2004

===============================================================================
*/

#pragma once

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#define APP_CLASSNAME       "Tanks!"

#define PORT_SERVER     28101
#define PORT_CLIENT     28110

#define SHORT_SWAP(a)   (((a&0xffff)>>8)|((a&0xffff)<<8))&0xffff
#define LONG_SWAP(a)    ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24)

#define BIG_SHORT(a)    SHORT_SWAP(a)

#define LITTLE_LONG(a)  (a)

//------------------------------------------------------------------------------
using byte = std::uint8_t;
using word = std::uint16_t;

#define OED_LIB

#include "oed_shared.h"
#include "oed_error.h"
#include "oed_files.h"
#include "oed_tools.h"

#include "cm_vector.h"
#include "cm_matrix.h"
#include "cm_color.h"
#include "cm_error.h"

class vMain
{
public:
    virtual result message (char const* message, ...) = 0;
};

extern vMain    *pMain;

//------------------------------------------------------------------------------
template<typename T, std::size_t Sz> constexpr std::size_t countof(T const (&)[Sz])
{
    return Sz;
}
