/*
===========================================================

Name    :   oed_error.h

Purpose :   error checking class

Modified:   11/03/2006

===========================================================
*/

#pragma once

#include "oed_shared.h"
#include <string.h>     // strncpy

#define ERROR_NONE              0x00
#define ERROR_FAIL              0x01
#define ERROR_BADPTR            0x02
#define ERROR_BADINDEX          0x04
#define ERROR_DUPLICATE         0x08
#define ERROR_NOTFOUND          0x10
