/*
===========================================================

Name    :   oed_shared.h

Purpose :   preprocessor and type definitions

Modified:   11/03/2006

===========================================================
*/

#pragma once

// pragmas

#pragma warning (disable:4244)  // 8 to 4 byte floating point truncation
#pragma warning (disable:4267)  // 8 to 4 byte integer truncation

// local includes

#include "oed_types.h"

// standard includes

#include <stdio.h>  // va_start va_end vsprintf
#include <stdarg.h> // va_start va_end vsprintf
#include <stdlib.h> // rand srand

#ifdef _DEBUG
#include <assert.h> // assert
#else
#define assert(exp) ((void)0)
#endif

/*
===========================================================

PREPROCESSOR DEFINITIONS

===========================================================
*/

#define MAX_STRING      1024
#define LONG_STRING     256
#define SHORT_STRING    32

// strings
#define STRING_EQUAL    0

// common ascii key strokes
#define K_TAB       9
#define K_ENTER     13
#define K_ESCAPE    27
#define K_SPACE     32
#define K_BACKSPACE 127

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401
#endif

#define BIT(a)  (1<<a)

#define MAKEID(d,c,b,a)                 ( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )

template<typename T> T deg2rad(T value) { return value * T(M_PI / 180.0); }
template<typename T> T rad2deg(T value) { return value * T(180.0 / M_PI); }

__forceinline float frand() { return ((float)((rand()&32767)*(1.0f/32767.0f))); }
__forceinline float crand() { return ((float)((rand()&32767)*(2.0f/32767.0f)-1.0f)); }

template<typename T> T square(T value) { return value * value; }

template<typename T> T clamp(T value, T min, T max) { return (value < min) ? min : (value > max) ? max : value; }

template<typename T> T min(T a, T b) { return a < b ? a : b; }
template<typename T> T max(T a, T b) { return a > b ? a : b; }

/*
===========================================================

TEXT FORMATTING

===========================================================
*/

static char *va (char const *szMessage, ...)
{
    static char     szStrings[16][MAX_STRING];
    static byte     iNextString;
    va_list     apList;
    char        *Ret;

    va_start( apList, szMessage );
    vsprintf( szStrings[iNextString], szMessage, apList );
    va_end( apList );

    Ret = szStrings[iNextString];
    iNextString = (iNextString + 1)%16;

    return Ret;
}
