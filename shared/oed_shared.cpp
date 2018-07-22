//  oed_shared.cpp
//

#include "shared.h"

////////////////////////////////////////////////////////////////////////////////
char *va (char const *szMessage, ...)
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
